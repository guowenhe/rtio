/*
*
* Copyright 2023 RTIO authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
 */

package devicetcp

import (
	"context"
	"errors"
	"io"
	"net"
	"rtio2/internal/deviceaccess/access_server/backendconn"
	"rtio2/pkg/config"
	dp "rtio2/pkg/deviceproto"
	"rtio2/pkg/rpcproto/deviceservice"
	"rtio2/pkg/rpcproto/deviceverifier"

	ru "rtio2/pkg/rtioutils"
	"rtio2/pkg/timekv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/rs/zerolog/log"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

const (
	HEARTBEAT_SECONDS_DEFAULT = 300
)

var (
	OutgoingChanSize = 10

	ErrRtioInternalServerError = errors.New("ErrRtioInternalServerError")
	ErrSessionNotFound         = errors.New("ErrSessionNotFound")
	ErrDataType                = errors.New("ErrDataType")
	ErrOverCapacity            = errors.New("ErrOverCapacity")
	ErrSendTimeout             = errors.New("ErrSendTimeout")
	ErrSendRespChannClose      = errors.New("ErrSendRespChannClose")
	ErrSessionAuthData         = errors.New("ErrSessionAuthData")
	ErrSessionAuthNotCompleted = errors.New("ErrSessionAuthNotCompleted")
	ErrSessionHeartbeatTimeout = errors.New("ErrSessionHeartbeatTimeout")
	ErrHeaderIDNotExist        = errors.New("ErrHeaderIDNotExist")
	ErrObservaNotMatch         = errors.New("ErrObservaNotMatch")
	ErrObservaNotFound         = errors.New("ErrObservaNotFound")
	ErrMethodNotAllowed        = errors.New("ErrMethodNotAllowed")
	ErrResourceNotFound        = errors.New("ErrResourceNotFound")
	ErrMethodNotMatch          = errors.New("ErrMethodNotMatch")
)

type Message struct {
	ID   uint32
	Data []byte
}

type Session struct {
	deviceID         string
	conn             net.Conn
	outgoingChan     chan []byte
	sendIDStore      *timekv.TimeKV
	observaStore     sync.Map
	rollingHeaderID  atomic.Uint32 // rolling number for header id
	rollingObservaID atomic.Uint32 // rolling number for observation id
	observaCount     atomic.Int32
	BodyCapSize      uint16
	heartbeatSeconds uint16
	remoteAddr       net.Addr
	authPass         bool
	cancel           context.CancelFunc
	done             chan struct{}
}

func newSession(conn net.Conn) *Session {
	s := &Session{
		conn:             conn,
		outgoingChan:     make(chan []byte, OutgoingChanSize),
		authPass:         false,
		done:             make(chan struct{}, 1),
		heartbeatSeconds: HEARTBEAT_SECONDS_DEFAULT,
	}
	s.sendIDStore = timekv.NewTimeKV(time.Second * 120)
	s.rollingHeaderID.Store(0)
	s.rollingObservaID.Store(0)
	s.observaCount.Store(0)
	return s
}

type Observa struct { // Observation
	ID              uint16
	FrameID         uint32
	NotifyChan      chan *dp.ObGetNotifyReq
	SessionDoneChan chan struct{}
}

func transToDeviceCode(code deviceservice.Code) dp.StatusCode {
	switch code {
	case deviceservice.Code_CODE_INTERNAL_SERVER_ERROR:
		return dp.StatusCode_InternalServerError
	case deviceservice.Code_CODE_OK:
		return dp.StatusCode_OK
	case deviceservice.Code_CODE_CONTINUE:
		return dp.StatusCode_Continue
	case deviceservice.Code_CODE_TERMINATE:
		return dp.StatusCode_Terminate
	case deviceservice.Code_CODE_NOT_FOUNT:
		return dp.StatusCode_NotFount
	case deviceservice.Code_CODE_BAD_REQUEST:
		return dp.StatusCode_BadRequest
	case deviceservice.Code_CODE_METHOD_NOT_ALLOWED:
		return dp.StatusCode_MethodNotAllowed
	case deviceservice.Code_CODE_TOO_MANY_REQUESTS:
		return dp.StatusCode_TooManyRequests
	case deviceservice.Code_CODE_TOO_MANY_OBSERVATIONS:
		return dp.StatusCode_TooManyObservations
	}
	log.Error().Str("rpc.status", code.String()).Msg("transferRPCStatus not mapped")
	return dp.StatusCode_Unknown
}

func calcuCheckSenconds(heartbeat uint16) time.Duration {

	return time.Duration(heartbeat + (heartbeat >> 1)) // heartbeat * 1.5
}

func (s *Session) genHeaderID() uint16 {
	return uint16(s.rollingHeaderID.Add(1))
}
func (s *Session) genObservaID() uint16 {
	return uint16(s.rollingObservaID.Add(1))
}
func (s *Session) CreateObserva() (*Observa, error) {
	if s.observaCount.Load() >= dp.OBGET_OBSERVATIONS_MAX {
		return nil, dp.StatusCode_TooManyObservations
	}
	ob := &Observa{
		ID:              s.genObservaID(),
		FrameID:         0,
		NotifyChan:      make(chan *dp.ObGetNotifyReq, 1),
		SessionDoneChan: make(chan struct{}, 1),
	}
	s.observaStore.Store(ob.ID, ob)
	s.observaCount.Add(1)
	log.Debug().Uint16("obid", ob.ID).Int32("obcount", s.observaCount.Load()).Msg("CreateObserva")
	return ob, nil
}
func (s *Session) DestroyObserva(id uint16) {
	if v, ok := s.observaStore.LoadAndDelete(id); ok {
		ob := v.(*Observa)
		close(ob.NotifyChan)
		s.observaCount.Add(-1)
		log.Debug().Uint16("obid", ob.ID).Int32("obcount", s.observaCount.Load()).Msg("DestroyObserva")
	}
}

func (s *Session) sendObEstabReq(ob *Observa, uri uint32, headerID uint16, data []byte) (<-chan []byte, error) {
	req := &dp.ObGetEstabReq{
		HeaderID: headerID,
		Method:   dp.Method_ObservedGet,
		ObID:     ob.ID,
		URI:      uri,
		Data:     data,
	}

	buf := make([]byte, dp.HeaderLen+dp.HeaderLen_ObGetEstabReq+uint16(len(data)))
	if err := dp.EncodeObGetEstabReq_OverServerSendReq(req, buf); err != nil {
		log.Error().Uint16("obid", ob.ID).Err(err).Msg("sendObEstabReq")
		return nil, err
	}

	respChan := make(chan []byte, 1)
	s.sendIDStore.Set(timekv.Key(headerID), &timekv.Value{C: respChan})
	s.outgoingChan <- buf
	return respChan, nil
}

func (s *Session) receiveObEstabResp(ob *Observa, headerID uint16, respChan <-chan []byte, timeout time.Duration) (dp.StatusCode, error) {

	defer s.sendIDStore.Del(timekv.Key(headerID))
	t := time.NewTimer(timeout)
	defer t.Stop()
	select {
	case sendRespBody, ok := <-respChan:
		if !ok {
			log.Error().Uint16("obid", ob.ID).Err(ErrSendRespChannClose).Msg("receiveObEstabResp")
			return dp.StatusCode_Unknown, ErrSendRespChannClose
		}
		obEstabResp, err := dp.DecodeObGetEstabResp(headerID, sendRespBody)
		if err != nil {
			log.Error().Uint16("obid", ob.ID).Err(err).Msg("receiveObEstabResp")
			return dp.StatusCode_Unknown, err
		}
		if obEstabResp.ObID != ob.ID {
			log.Error().Uint16("obid", ob.ID).Uint16("obid", ob.ID).Uint16("resp.obid", obEstabResp.ObID).Err(ErrObservaNotMatch).Msg("receiveObEstabResp")
			return dp.StatusCode_Unknown, ErrObservaNotMatch
		}
		if obEstabResp.Method != dp.Method_ObservedGet {
			log.Error().Uint16("obid", ob.ID).Err(dp.StatusCode_MethodNotAllowed).Msg("receiveObEstabResp")
			return dp.StatusCode_MethodNotAllowed, nil
		}
		return obEstabResp.Code, nil
	case <-t.C:
		log.Error().Uint16("obid", ob.ID).Err(ErrSendTimeout).Msg("Send")
		return dp.StatusCode_Unknown, ErrSendTimeout
	}
}

func (s *Session) receiveObNotifyReq(header *dp.Header, buf []byte) error {
	req, err := dp.DecodeObGetNotifyReq(header.ID, buf)
	if err != nil {
		log.Error().Uint16("obid", req.ObID).Err(err).Msg("receiveObNotifyReq")
		return err
	}

	resp := &dp.ObGetNotifyResp{
		HeaderID: req.HeaderID,
		Method:   req.Method,
		ObID:     req.ObID,
	}
	if ob, ok := s.observaStore.Load(req.ObID); !ok {
		// observation ID not found, means observer not interesting, terminate observation
		resp.Code = dp.StatusCode_Terminate
		log.Info().Uint16("obid", req.ObID).Msg("receiveObNotifyReq not found observa, terminate it")
	} else {
		resp.Code = dp.StatusCode_Continue
		ob.(*Observa).NotifyChan <- req
	}

	if err := s.sendObNotifyResp(resp); err != nil {
		log.Error().Uint16("obid", req.ObID).Err(err).Msg("receiveObNotifyReq")
	}
	return nil
}

func (s *Session) sendObNotifyResp(resp *dp.ObGetNotifyResp) error {
	log.Info().Uint16("obid", resp.ObID).Uint16("headerid", resp.HeaderID).Msg("SendObNotifyResp")
	buf := make([]byte, dp.HeaderLen+dp.HeaderLen_ObGetNotifyResp)
	if err := dp.EncodeObGetNotifyResp_OverDeviceSendResp(resp, buf); err != nil {
		log.Error().Uint16("obid", resp.ObID).Err(err).Msg("SendObNotifyResp")
		return err
	}
	s.outgoingChan <- buf
	return nil
}
func (s *Session) ObGetEstablish(ctx context.Context, uri uint32, ob *Observa, data []byte, timeout time.Duration) (dp.StatusCode, error) {

	headerID := s.genHeaderID()
	respChan, err := s.sendObEstabReq(ob, uri, headerID, data)
	if err != nil {
		return dp.StatusCode_Unknown, err
	}
	statusCode, err := s.receiveObEstabResp(ob, headerID, respChan, timeout)
	if err != nil {
		return dp.StatusCode_Unknown, err
	}
	return statusCode, nil
}

func (s *Session) sendCoReq(uri uint32, method dp.Method, headerID uint16, data []byte) (<-chan []byte, error) {
	req := &dp.CoReq{
		HeaderID: headerID,
		Method:   method,
		URI:      uri,
		Data:     data,
	}
	log.Info().Uint16("headerid", headerID).Uint32("uri", uri).Msg("sendCoReq")
	buf := make([]byte, dp.HeaderLen+dp.HeaderLen_CoReq+uint16(len(data)))
	if err := dp.EncodeCoReq_OverServerSendReq(req, buf); err != nil {
		log.Error().Uint16("headerid", headerID).Err(err).Msg("sendCoReq")
		return nil, err
	}
	respChan := make(chan []byte, 1)
	s.sendIDStore.Set(timekv.Key(headerID), &timekv.Value{C: respChan})
	s.outgoingChan <- buf
	return respChan, nil
}

func (s *Session) receiveCoResp(headerID uint16, respChan <-chan []byte, timeout time.Duration) (dp.StatusCode, []byte, error) {
	defer s.sendIDStore.Del(timekv.Key(headerID))
	t := time.NewTimer(timeout)
	defer t.Stop()
	select {
	case sendRespBody, ok := <-respChan:
		if !ok {
			log.Error().Err(ErrSendRespChannClose).Msg("receiveCoResp")
			return dp.StatusCode_Unknown, nil, ErrSendRespChannClose
		}
		coResp, err := dp.DecodeCoResp(headerID, sendRespBody)
		if err != nil {
			log.Error().Err(err).Msg("receiveCoResp")
			return dp.StatusCode_Unknown, nil, err
		}
		if coResp.Method != dp.Method_ConstrainedGet && coResp.Method != dp.Method_ConstrainedPost {
			log.Error().Err(ErrMethodNotMatch).Msg("receiveCoResp")
			return dp.StatusCode_InternalServerError, nil, ErrMethodNotMatch
		}
		log.Info().Uint16("headerid", coResp.HeaderID).Str("status", coResp.Code.String()).Msg("receiveCoResp")

		return coResp.Code, coResp.Data, nil
	case <-t.C:
		log.Error().Err(ErrSendTimeout).Msg("receiveCoResp")
		return dp.StatusCode_Unknown, nil, ErrSendTimeout
	}
}

func (s *Session) Send(ctx context.Context, uri uint32, method dp.Method, data []byte, timeout time.Duration) (dp.StatusCode, []byte, error) {
	headerID := s.genHeaderID()
	respChan, err := s.sendCoReq(uri, method, headerID, data)
	if err != nil {
		return dp.StatusCode_Unknown, nil, err
	}
	statusCode, data, err := s.receiveCoResp(headerID, respChan, timeout)
	if err != nil {
		return dp.StatusCode_Unknown, nil, err
	}
	return statusCode, data, nil
}

func (s *Session) Cancel() {
	if nil == s.cancel {
		log.Error().Msg("Session cancel is nil")
	}
	s.cancel()
}
func (s *Session) Done() <-chan struct{} {
	return s.done
}
func (s *Session) receiveAuthReq(ctx context.Context, header *dp.Header) (bool, error) {
	reqBodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, reqBodyBuf)
	if err != nil {
		log.Error().Int("readLen", readLen).Err(err).Msg("receiveAuthReq, ReadFull")
		return false, err
	}
	req, err := dp.DecodeAuthReqBody(header, reqBodyBuf)
	if err != nil {
		log.Error().Err(err).Msg("receiveAuthReq, DecodeBodyAuthReq")
		return false, err
	}
	if len(req.DeviceID) == 0 || len(req.DeviceSecret) == 0 {
		log.Error().Err(ErrSessionAuthData).Msg("receiveAuthReq, DecodeBodyAuthReq")
		return false, s.sendAuthResp(header, dp.Code_ParaInvalid)
	}
	// device verify
	if !config.BoolKV.GetWithDefault("disable.deviceverify", false) {
		authClient, err := backendconn.GetDeviceVerifierClient()
		if err != nil {
			log.Error().Err(err).Msg("GetDeviceVerifierClient err")
			return false, s.sendAuthResp(header, dp.Code_UnkownErr)
		}

		id, err := ru.GenUint32ID()
		if err != nil {
			log.Error().Err(err).Msg("GenUint32ID err")
			return false, s.sendAuthResp(header, dp.Code_UnkownErr)
		}
		verifierReq := &deviceverifier.VerifyReq{
			Id:           id,
			DeviceId:     req.DeviceID,
			DeviceSecret: req.DeviceSecret,
		}
		VerifyResp, err := authClient.Verify(ctx, verifierReq)
		if err != nil {
			log.Error().Err(err).Msg("call Verify err")
			return s.authPass, s.sendAuthResp(header, dp.Code_UnkownErr)
		}
		if VerifyResp.Code != deviceverifier.Code_CODE_PASS {
			log.Error().Err(err).Str("deviceverifiercode", VerifyResp.Code.String()).Msg("receiveAuthReq")
			return false, s.sendAuthResp(header, dp.Code_AuthFail)
		}
	}
	// auth pass
	s.authPass = true
	s.deviceID = req.DeviceID
	capSize, err := dp.GetCapSize(req.CapLevel)
	if err != nil {
		return true, s.sendAuthResp(header, dp.Code_Success)
	}
	s.BodyCapSize = capSize
	return true, nil
}

func (s *Session) sendAuthResp(header *dp.Header, code dp.RemoteCode) error {
	resp := &dp.AuthResp{
		Header: &dp.Header{
			Version: dp.Version,
			Type:    dp.MsgType_DeviceAuthResp,
			ID:      header.ID,
			BodyLen: 0,
			Code:    code,
		},
	}
	respBuf, err := dp.EncodeAuthResp(resp)
	if err != nil {
		log.Error().Err(err).Msg("deviceSendAuthResp")
		return err
	}
	s.outgoingChan <- respBuf
	return nil
}

func (s *Session) devicePingProcess(header *dp.Header) error {

	bodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, bodyBuf)
	if err != nil {
		log.Error().Int("readLen", readLen).Err(err).Msg("devicePingProcess, ReadFull")
		return err
	}

	respCode := dp.Code_Success
	req, err := dp.DecodePingReqBody(header, bodyBuf)
	if err != nil {
		log.Error().Err(err).Msg("devicePingProcess, DecodePingReqBody")
		if err == dp.ErrLengthError {
			respCode = dp.Code_LengthErr
		} else {
			return err
		}
	}
	// update timeout value by device
	if req.Timeout != 0 {
		if req.Timeout < 30 || req.Timeout > 43200 { // 43200 secodes = 12 hours
			respCode = dp.Code_ParaInvalid
		} else {
			s.heartbeatSeconds = req.Timeout
		}
	}

	// send ping resp
	resp := &dp.PingResp{
		Header: &dp.Header{
			Version: dp.Version,
			Type:    dp.MsgType_DevicePingResp,
			ID:      req.Header.ID,
			BodyLen: 0,
			Code:    respCode,
		},
	}
	respBuf, err := dp.EncodePingResp(resp)
	if err != nil {
		return err
	}
	s.outgoingChan <- respBuf
	return nil
}
func (s *Session) serverSendRespone(header *dp.Header) error {
	value, ok := s.sendIDStore.Get(timekv.Key(header.ID))

	if !ok {
		log.Warn().Uint16("headerid", header.ID).Err(ErrHeaderIDNotExist).Msg("serverSendRespone")
		return ErrHeaderIDNotExist
	}

	bodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, bodyBuf)
	if err != nil {
		log.Error().Int("readLen", readLen).Err(err).Msg("serverSendRespone, ReadFull")
		return err
	}

	sendResp, err := dp.DecodeSendRespBody(header, bodyBuf)
	if err != nil {
		log.Error().Err(err).Msg("serverSendRespone, DecodeBodySendResp")
		return err
	}
	value.C <- sendResp.Body
	return nil
}

func (s *Session) accessResource(method dp.Method, req *deviceservice.Req) (*deviceservice.Resp, error) {

	client, err := backendconn.GetDeviceServiceClient()
	if err != nil {
		log.Error().Err(err).Msg("accessResource")
		return nil, ErrResourceNotFound
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
	defer cancel()
	var resp *deviceservice.Resp
	if method == dp.Method_ConstrainedPost {
		resp, err = client.Post(ctx, req)
		if err != nil {
			if status.Code(err) == codes.Unimplemented {
				log.Warn().Err(err).Msg("accessResource")
				return nil, ErrMethodNotAllowed
			}
			log.Error().Err(err).Msg("accessResource, Post error")
			return nil, err
		}
	} else if method == dp.Method_ConstrainedGet {
		resp, err = client.Get(ctx, req)
		if err != nil {
			if status.Code(err) == codes.Unimplemented {
				log.Warn().Err(err).Msg("accessResource")
				return nil, ErrMethodNotAllowed
			}
			log.Error().Err(err).Msg("accessResource, Get error")
			return nil, err
		}
	} else {
		log.Error().Err(ErrMethodNotAllowed).Msg("accessResource error")
		return nil, ErrMethodNotAllowed
	}
	return resp, nil
}
func (s *Session) receiveCoReq(header *dp.Header, buf []byte) error {
	req, err := dp.DecodeCoReq(header.ID, buf)
	if err != nil {
		log.Error().Err(err).Msg("receiveCoReq")
		return err
	}
	resp := &dp.CoResp{
		HeaderID: req.HeaderID,
		Method:   req.Method,
	}

	if config.BoolKV.GetWithDefault("disable.deviceservice", false) {
		resp.Code = dp.StatusCode_MethodNotAllowed
	} else {
		id, err := ru.GenUint32ID()
		if err != nil {
			log.Error().Err(err).Msg("receiveCoReq, GenUint32ID error")
			resp.Code = dp.StatusCode_InternalServerError
		}
		resReq := &deviceservice.Req{
			Id:       id,
			Uri:      req.URI,
			DeviceId: s.deviceID,
			Data:     req.Data,
		}
		log.Info().Uint16("headerid", req.HeaderID).Uint32("serialid", id).Uint32("uri", req.URI).Uint8("method", uint8(req.Method)).Msg("receiveCoReq")
		resResp, err := s.accessResource(req.Method, resReq)
		if err != nil {
			if err == ErrMethodNotAllowed {
				resp.Code = dp.StatusCode_MethodNotAllowed
			} else if err == ErrResourceNotFound {
				resp.Code = dp.StatusCode_NotFount
			} else {
				resp.Code = dp.StatusCode_InternalServerError
			}
		} else {
			resp.Code = transToDeviceCode(resResp.Code)
			if resp.Code == dp.StatusCode_OK {
				resp.Data = resResp.Data
			}
		}
	}

	if err := s.sendCoResp(resp); err != nil {
		log.Error().Err(err).Msg("receiveCoReq")
	}
	return nil
}
func (s *Session) sendCoResp(resp *dp.CoResp) error {
	log.Info().Uint16("headerid", resp.HeaderID).Str("status", resp.Code.String()).Msg("sendCoResp")
	buf := make([]byte, dp.HeaderLen+dp.HeaderLen_CoResp+uint16(len(resp.Data)))
	if err := dp.EncodeCoResp_OverDeviceSendResp(resp, buf); err != nil {
		log.Error().Err(err).Msg("sendCoResp")
		return err
	}
	s.outgoingChan <- buf
	return nil
}

func (s *Session) deviceSendRequest(header *dp.Header) error {

	bodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, bodyBuf)
	if err != nil {
		log.Error().Int("readLen", readLen).Err(err).Msg("deviceSendRequest, ReadFull")
		return err
	}

	method, err := dp.DecodeMethod(bodyBuf)
	if err != nil {
		log.Error().Err(err).Msg("deviceSendRequest")
		return err
	}

	switch method {
	case dp.Method_ConstrainedGet, dp.Method_ConstrainedPost:
		err := s.receiveCoReq(header, bodyBuf)
		if err != nil {
			log.Error().Err(err).Msg("deviceSendRequest")
			return err
		}
	case dp.Method_ObservedGet:
		err := s.receiveObNotifyReq(header, bodyBuf)
		if err != nil {
			log.Error().Err(err).Msg("deviceSendRequest")
			return err
		}
	}
	return nil
}
func (s *Session) tcpIncomming(serveCtx context.Context, addSession func(string, *Session),
	authTimer *time.Timer, heartbeatTimer *time.Ticker, errChan chan<- error) {
	defer func() {
		log.Debug().Msg("tcpIncomming exit")
	}()

	for {
		select {
		case <-serveCtx.Done():
			log.Debug().Msg("tcpIncomming context done")
			return
		default:

			headBuf := make([]byte, dp.HeaderLen)
			readLen, err := io.ReadFull(s.conn, headBuf)
			log.Debug().Int("readlen", readLen).Hex("buf", headBuf).Msg("tcpIncomming, read header")
			if err != nil {
				errChan <- err
				return
			}
			header, err := dp.DecodeHeader(headBuf)
			if err != nil {
				errChan <- dp.ErrDecode
				return
			}

			switch header.Type {
			case dp.MsgType_DeviceAuthReq:
				if ok, err := s.receiveAuthReq(serveCtx, header); err != nil {
					errChan <- err
					return
				} else {
					if ok { // auth passed
						addSession(s.deviceID, s)
						authTimer.Stop()
						log.Debug().Msg("tcpIncomming, auth passed, authTimer stoped")
					}
				}
			case dp.MsgType_DevicePingReq:
				if err := s.devicePingProcess(header); err != nil {
					errChan <- err
					return
				}
				log.Debug().Uint16("heartbeat", s.heartbeatSeconds).Msg("tcpIncomming, ping req")
				heartbeatTimer.Reset(time.Second * calcuCheckSenconds(s.heartbeatSeconds))
			case dp.MsgType_DeviceSendReq:
				if err := s.deviceSendRequest(header); err != nil {
					errChan <- err
					return
				}
				heartbeatTimer.Reset(time.Second * calcuCheckSenconds(s.heartbeatSeconds))
			case dp.MsgType_ServerSendResp:
				if err := s.serverSendRespone(header); err != nil {
					errChan <- err
					return
				}
				heartbeatTimer.Reset(time.Second * calcuCheckSenconds(s.heartbeatSeconds))
			default:
				log.Error().Err(ErrDataType).Uint8("type", uint8(header.Type)).Msg("tcpIncomming")
				errChan <- ErrDataType
				return
			}

		}
	}
}
func (s *Session) tcpOutgoing(serveCtx context.Context, errChan chan<- error) {
	defer func() {
		log.Debug().Msg("tcpOutgoing exit")
	}()

	for {
		select {

		case <-serveCtx.Done():
			log.Debug().Msg("tcpOutgoing context done")
			return
		case buf := <-s.outgoingChan:
			if n, err := ru.WriteFull(s.conn, buf); err != nil {
				log.Error().Err(err).Int("writelen", n).Int("buflen", len(buf)).Msg("tcpOutgoing WriteFull error")
				errChan <- err
				return
			}
		}
	}
}
func (s *Session) serve(ctx context.Context, wait *sync.WaitGroup,
	addSession func(string, *Session), delSession func(string)) {
	var serveCtx context.Context
	serveCtx, s.cancel = context.WithCancel(ctx)
	serveLogger := log.With().Str("client_ip", s.conn.RemoteAddr().String()).Logger()
	serveLogger.Info().Msg("start serving")
	defer func() {
		serveLogger.Info().Msg("stop service for this conn")
	}()

	s.remoteAddr = s.conn.RemoteAddr()

	defer wait.Done()
	defer s.conn.Close()
	defer func() {
		// close all observations
		s.observaStore.Range(func(k, v any) bool {
			log.Info().Uint16("obid", k.(uint16)).Msg("send done observation")
			v.(*Observa).SessionDoneChan <- struct{}{}
			return true
		})
		// delete session when auth pass and session be created
		if s.authPass {
			delSession(s.deviceID)
		}
		s.done <- struct{}{}
	}()

	authTimer := time.NewTimer(time.Second * 15)
	defer authTimer.Stop()
	heartbeatTimer := time.NewTicker(time.Second * calcuCheckSenconds(s.heartbeatSeconds))
	defer heartbeatTimer.Stop()
	errChan := make(chan error, 2)
	go s.tcpOutgoing(serveCtx, errChan)
	go s.tcpIncomming(serveCtx, addSession, authTimer, heartbeatTimer, errChan)

	storeTicker := time.NewTicker(time.Second * 5)
	defer storeTicker.Stop()

	for {
		select {
		case <-authTimer.C:
			log.Debug().Err(ErrSessionAuthNotCompleted).Msg("authTimer timeout")
			errChan <- ErrSessionAuthNotCompleted
			return
		case <-heartbeatTimer.C:
			log.Debug().Err(ErrSessionHeartbeatTimeout).Msg("tcpIncomming heartbeatTimer timeout")
			errChan <- ErrSessionHeartbeatTimeout
			return
		case err := <-errChan:
			s.cancel()
			log.Warn().Err(err).Msg("serve done when error")
			return
		case <-serveCtx.Done():
			log.Debug().Msg("serve done when ctx done")
			if err := s.conn.Close(); err != nil {
				log.Debug().Err(err).Msg("sever close conn error")
			}
			return
		case <-storeTicker.C:
			// servelogger.Debug().Msgf("storeTicker.C %s", time.Now().String())
			s.sendIDStore.DelExpireKeys()
		}
	}
}

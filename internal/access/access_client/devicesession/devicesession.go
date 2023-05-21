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

package devicesession

import (
	"context"
	"errors"
	"io"
	"net"
	dp "rtio2/pkg/deviceproto"
	"rtio2/pkg/log"
	ru "rtio2/pkg/rtioutils"
	"rtio2/pkg/timekv"
	"sync/atomic"
	"time"
)

var logger log.Logger

func init() {
	logger = log.With().Str("module", "device").Logger()
}

var (
	ErrDataType           = errors.New("ErrDataType")
	ErrOverCapacity       = errors.New("ErrOverCapacity")
	ErrSendTimeout        = errors.New("ErrSendTimeout")
	ErrAuthFailed         = errors.New("ErrAuthFailed")
	ErrTransFunc          = errors.New("ErrTransFunc")
	ErrObservaNotMatch    = errors.New("ErrObservaNotMatch")
	ErrMethodNotMatch     = errors.New("ErrMethodNotMatch")
	ErrSendRespChannClose = errors.New("ErrSendRespChannClose")
	ErrHeaderIDNotExist   = errors.New("ErrHeaderIDNotExist")
)

type DeviceSession struct {
	deviceID           string
	deviceSecret       string
	ssframeChan        chan *SSFrame
	outgoingChan       chan []byte
	regGetHandlerMap   map[uint32]func(req []byte) ([]byte, error)
	regPostHandlerMap  map[uint32]func(req []byte) ([]byte, error)
	regObGetHandlerMap map[uint32]func(ctx context.Context, req []byte) (<-chan []byte, error)
	sendIDStore        *timekv.TimeKV
	rollingHeaderID    atomic.Uint32 // rolling number for header id
	conn               net.Conn
}

type SSFrame struct { // Server Send Frame
	ID    uint16
	data  []byte
	trans func([]byte)
}

func NewSSFrame(id uint16, data []byte, trans func([]byte)) *SSFrame {
	return &SSFrame{
		ID:    id,
		data:  data,
		trans: trans,
	}
}
func (f *SSFrame) GetRequest() []byte {
	return f.data
}
func (f *SSFrame) SetResponse(data []byte) error {
	resp := &dp.SendResp{
		Header: &dp.Header{
			Version: dp.Version,
			ID:      f.ID,
			Type:    dp.MsgType_ServerSendResp,
			BodyLen: 0, // auto update when endoce
			Code:    dp.Code_Success,
		},
		Body: data,
	}

	buf, err := dp.EncodeSendResp(resp)
	if err != nil {
		logger.Error().Err(err).Msg("SetResponse")
		return err
	}

	if nil == f.trans {
		logger.Error().Err(ErrTransFunc).Msg("SetResponse")
		return ErrTransFunc
	}
	f.trans(buf)
	return nil
}

func NewDeviceSession(conn net.Conn, deviceId, deviceSecret string) *DeviceSession {
	s := &DeviceSession{
		deviceID:           deviceId,
		deviceSecret:       deviceSecret,
		ssframeChan:        make(chan *SSFrame, 10),
		outgoingChan:       make(chan []byte, 10),
		sendIDStore:        timekv.NewTimeKV(time.Second * 120),
		regGetHandlerMap:   make(map[uint32]func(req []byte) ([]byte, error), 1),
		regPostHandlerMap:  make(map[uint32]func(req []byte) ([]byte, error), 1),
		regObGetHandlerMap: make(map[uint32]func(ctx context.Context, req []byte) (<-chan []byte, error), 1),
		conn:               conn,
	}
	s.rollingHeaderID.Store(0)
	return s
}

func (s *DeviceSession) genHeaderID() uint16 {
	return uint16(s.rollingHeaderID.Add(1))
}

func (s *DeviceSession) auth() error {
	headerID, err := ru.GenUint16ID()
	if err != nil {
		return err
	}
	req := &dp.AuthReq{
		Header: &dp.Header{
			Version: dp.Version,
			Type:    dp.MsgType_DeviceAuthReq,
			ID:      headerID,
		},
		CapLevel:     1,
		DeviceID:     s.deviceID,
		DeviceSecret: s.deviceSecret,
	}

	buf, err := dp.EncodeAuthReq(req)
	if err != nil {
		return err
	}
	s.outgoingChan <- buf
	return nil
}
func (s *DeviceSession) sendCoReq(headerID uint16, method dp.Method, uri uint32, data []byte) (<-chan []byte, error) {
	req := &dp.CoReq{
		HeaderID: headerID,
		Method:   method,
		URI:      uri,
		Data:     data,
	}
	logger.Info().Uint16("headerid", headerID).Uint32("uri", uri).Msg("sendCoReq")
	buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_CoReq)+len(data))
	if err := dp.EncodeCoReq_OverDeviceSendReq(req, buf); err != nil {
		logger.Error().Uint16("headerid", headerID).Err(err).Msg("sendCoReq")
		return nil, err
	}
	respChan := make(chan []byte, 1)
	s.sendIDStore.Set(timekv.Key(headerID), &timekv.Value{C: respChan})
	s.outgoingChan <- buf
	return respChan, nil
}
func (s *DeviceSession) receiveCoResp(headerID uint16, respChan <-chan []byte, timeout time.Duration) (dp.StatusCode, []byte, error) {
	defer s.sendIDStore.Del(timekv.Key(headerID))
	t := time.NewTimer(timeout)
	defer t.Stop()
	select {
	case sendRespBody, ok := <-respChan:
		if !ok {
			logger.Error().Err(ErrSendRespChannClose).Msg("receiveCoResp")
			return dp.StatusCode_Unknown, nil, ErrSendRespChannClose
		}
		coResp, err := dp.DecodeCoResp(headerID, sendRespBody)
		if err != nil {
			logger.Error().Err(err).Msg("receiveCoResp")
			return dp.StatusCode_Unknown, nil, err
		}
		if coResp.Method != dp.Method_ConstrainedGet && coResp.Method != dp.Method_ConstrainedPost {
			logger.Error().Err(ErrMethodNotMatch).Msg("receiveCoResp")
			return dp.StatusCode_InternalServerError, nil, ErrMethodNotMatch
		}
		logger.Info().Uint16("headerid", coResp.HeaderID).Str("status", coResp.Code.String()).Msg("receiveCoResp")

		return coResp.Code, coResp.Data, nil
	case <-t.C:
		logger.Error().Err(ErrSendTimeout).Msg("receiveCoResp")
		return dp.StatusCode_Unknown, nil, ErrSendTimeout
	}
}

func (s *DeviceSession) sendObNotifyReq(obid uint16, headerID uint16, data []byte) (<-chan []byte, error) {
	req := &dp.ObGetNotifyReq{
		HeaderID: headerID,
		ObID:     obid,
		Method:   dp.Method_ObservedGet,
		Code:     dp.StatusCode_Continue,
		Data:     data,
	}
	logger.Info().Uint16("obid", obid).Uint16("headerid", headerID).Msg("sendObNotifyReq")
	buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_ObGetNotifyReq)+len(data))
	if err := dp.EncodeObGetNotifyReq_OverDeviceSendReq(req, buf); err != nil {
		return nil, err
	}
	respChan := make(chan []byte, 1)
	s.sendIDStore.Set(timekv.Key(headerID), &timekv.Value{C: respChan})
	s.outgoingChan <- buf
	return respChan, nil
}
func (s *DeviceSession) sendObNotifyReqTerminate(obid uint16, headerID uint16) (<-chan []byte, error) {
	req := &dp.ObGetNotifyReq{
		HeaderID: headerID,
		ObID:     obid,
		Method:   dp.Method_ObservedGet,
		Code:     dp.StatusCode_Terminate,
	}
	logger.Info().Uint16("obid", obid).Uint16("headerid", headerID).Msg("sendObNotifyReqTerminate")
	buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_ObGetNotifyReq))
	if err := dp.EncodeObGetNotifyReq_OverDeviceSendReq(req, buf); err != nil {
		return nil, err
	}
	respChan := make(chan []byte, 1)
	s.sendIDStore.Set(timekv.Key(headerID), &timekv.Value{C: respChan})
	s.outgoingChan <- buf
	return respChan, nil
}
func (s *DeviceSession) receiveObNotifyResp(obID, headerID uint16, respChan <-chan []byte, timeout time.Duration) (dp.StatusCode, error) {
	defer s.sendIDStore.Del(timekv.Key(headerID))
	t := time.NewTimer(timeout)
	defer t.Stop()
	select {
	case sendRespBody, ok := <-respChan:
		if !ok {
			logger.Error().Uint16("obid", obID).Err(ErrSendRespChannClose).Msg("receiveObNotifyResp")
			return dp.StatusCode_Unknown, ErrSendRespChannClose
		}
		obEstabResp, err := dp.DecodeObGetNotifyResp(headerID, sendRespBody)
		if err != nil {
			logger.Error().Uint16("obid", obID).Err(err).Msg("receiveObNotifyResp")
			return dp.StatusCode_Unknown, err
		}
		if obEstabResp.ObID != obID {
			logger.Error().Uint16("obid", obID).Uint16("resp.obid", obEstabResp.ObID).Err(ErrObservaNotMatch).Msg("receiveObNotifyResp")
			return dp.StatusCode_Unknown, ErrObservaNotMatch
		}
		if obEstabResp.Method != dp.Method_ObservedGet {
			logger.Error().Uint16("obid", obID).Err(dp.StatusCode_MethodNotAllowed).Msg("receiveObNotifyResp")
			return dp.StatusCode_MethodNotAllowed, nil
		}
		return obEstabResp.Code, nil
	case <-t.C:
		logger.Error().Uint16("obid", obID).Err(ErrSendTimeout).Msg("receiveObNotifyResp")
		return dp.StatusCode_Unknown, ErrSendTimeout
	}
}
func (s *DeviceSession) observe(obid uint16, cancal context.CancelFunc, notifyReqChan <-chan []byte) {
	logger.Info().Uint16("obid", obid).Msg("observe")
	defer func() {
		logger.Info().Uint16("obid", obid).Msg("observe exit")
	}()
	defer cancal()

	for {
		data, ok := <-notifyReqChan
		headerID := s.genHeaderID()
		if ok {
			notfyRespChan, err := s.sendObNotifyReq(obid, headerID, data)
			if err != nil {
				logger.Error().Uint16("obid", obid).Err(err).Msg("sendObNotifyReq error")
				return
			}
			statusCode, err := s.receiveObNotifyResp(obid, headerID, notfyRespChan, time.Second*20)
			if err != nil {
				logger.Error().Uint16("obid", obid).Err(err).Msg("receiveObNotifyResp error")
				return
			}
			if dp.StatusCode_Continue != statusCode {
				logger.Debug().Uint16("obid", obid).Str("status", statusCode.String()).Msg("observe terminiate")
				return
			}
		} else {
			logger.Debug().Uint16("obid", obid).Msg("observe, notify req channel closed")
			notfyRespChan, err := s.sendObNotifyReqTerminate(obid, headerID)
			if err != nil {
				logger.Error().Uint16("obid", obid).Err(err).Msg("sendObNotifyReqTerminate error")
				return
			}
			_, err = s.receiveObNotifyResp(obid, headerID, notfyRespChan, time.Second*20)
			if err != nil {
				logger.Error().Uint16("obid", obid).Err(err).Msg("receiveObNotifyResp error")
				return
			}
			return
		}
	}
}

func (s *DeviceSession) obGetReqHandler(header *dp.Header, reqBuf []byte) error {
	req, err := dp.DecodeObGetEstabReq(header.ID, reqBuf)
	if err != nil {
		logger.Error().Uint16("obid", req.ObID).Err(err).Msg("obGetReqHandler")
		return err
	}

	resp := &dp.ObGetEstabResp{
		HeaderID: req.HeaderID,
		Method:   req.Method,
		ObID:     req.ObID,
	}
	handler, ok := s.regObGetHandlerMap[req.URI]
	respBuf := make([]byte, dp.HeaderLen+dp.HeaderLen_ObGetEstabResp)
	if !ok {
		logger.Warn().Uint16("obid", req.ObID).Uint32("uri", req.URI).Msg("obGetReqHandler, uri not found")
		resp.Code = dp.StatusCode_NotFount
		if err := dp.EncodeObGetEstabResp_OverServerSendResp(resp, respBuf); err != nil {
			logger.Error().Uint16("obid", req.ObID).Err(err).Msg("obGetReqHandler")
			return err
		}
	} else {
		ctx, cancle := context.WithCancel(context.Background())
		respChan, err := handler(ctx, req.Data)
		if err != nil {
			resp.Code = dp.StatusCode_InternalServerError
			cancle()
			return err
		}
		resp.Code = dp.StatusCode_Continue
		if err := dp.EncodeObGetEstabResp_OverServerSendResp(resp, respBuf); err != nil {
			logger.Error().Uint16("obid", req.ObID).Err(err).Msg("obGetReqHandler")
			cancle()
			return err
		}
		go s.observe(req.ObID, cancle, respChan)
	}
	s.outgoingChan <- respBuf
	return nil
}

func (s *DeviceSession) coGetReqHandler(header *dp.Header, reqBuf []byte) error {
	req, err := dp.DecodeCoReq(header.ID, reqBuf)
	if err != nil {
		logger.Error().Err(err).Msg("coGetReqHandler")
		return err
	}
	resp := &dp.CoResp{
		HeaderID: req.HeaderID,
		Method:   req.Method,
	}
	handler, ok := s.regGetHandlerMap[req.URI]

	if !ok {
		logger.Warn().Uint16("headerid", req.HeaderID).Uint32("uri", req.URI).Msg("coGetReqHandler, uri not found")
		resp.Code = dp.StatusCode_NotFount
		buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_CoReq))
		if err := dp.EncodeCoResp_OverServerSendResp(resp, buf); err != nil {
			logger.Error().Uint16("headerid", req.HeaderID).Err(err).Msg("coGetReqHandler")
			return err
		}
		s.outgoingChan <- buf
	} else {
		data, err := handler(req.Data)
		if err != nil {
			resp.Code = dp.StatusCode_InternalServerError
			return err
		}
		resp.Code = dp.StatusCode_OK
		resp.Data = data
		buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_CoResp)+len(resp.Data))
		if err := dp.EncodeCoResp_OverServerSendResp(resp, buf); err != nil {
			logger.Error().Uint16("headerid", req.HeaderID).Err(err).Msg("coGetReqHandler")
			return err
		}
		s.outgoingChan <- buf
	}
	return nil
}

func (s *DeviceSession) coPostReqHandler(header *dp.Header, reqBuf []byte) error {
	req, err := dp.DecodeCoReq(header.ID, reqBuf)
	if err != nil {
		logger.Error().Err(err).Msg("coPostReqHandler")
		return err
	}
	resp := &dp.CoResp{
		HeaderID: req.HeaderID,
		Method:   req.Method,
	}
	handler, ok := s.regPostHandlerMap[req.URI]

	if !ok {
		logger.Warn().Uint16("headerid", req.HeaderID).Uint32("uri", req.URI).Msg("coGetReqHandler, uri not found")
		resp.Code = dp.StatusCode_NotFount
		buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_CoReq))
		if err := dp.EncodeCoResp_OverServerSendResp(resp, buf); err != nil {
			logger.Error().Uint16("headerid", req.HeaderID).Err(err).Msg("coGetReqHandler")
			return err
		}
		s.outgoingChan <- buf
	} else {
		data, err := handler(req.Data)
		if err != nil {
			resp.Code = dp.StatusCode_InternalServerError
			return err
		}
		resp.Code = dp.StatusCode_OK
		resp.Data = data
		buf := make([]byte, int(dp.HeaderLen+dp.HeaderLen_CoResp)+len(resp.Data))
		if err := dp.EncodeCoResp_OverServerSendResp(resp, buf); err != nil {
			logger.Error().Uint16("headerid", req.HeaderID).Err(err).Msg("coGetReqHandler")
			return err
		}
		s.outgoingChan <- buf
	}
	return nil
}

func (s *DeviceSession) serverSendRequest(header *dp.Header) error {

	bodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, bodyBuf)
	if err != nil {
		logger.Error().Int("readLen", readLen).Err(err).Msg("serverSendRequest, ReadFull")
		return err
	}

	method, err := dp.DecodeMethod(bodyBuf)
	if err != nil {
		logger.Error().Err(err).Msg("serverSendRequest")
		return err
	}

	switch method {
	case dp.Method_ObservedGet:
		err := s.obGetReqHandler(header, bodyBuf)
		if err != nil {
			logger.Error().Err(err).Msg("serverSendRequest")
			return err
		}
	case dp.Method_ConstrainedGet:
		err := s.coGetReqHandler(header, bodyBuf)
		if err != nil {
			logger.Error().Err(err).Msg("serverSendRequest")
			return err
		}
	case dp.Method_ConstrainedPost:
		err := s.coPostReqHandler(header, bodyBuf)
		if err != nil {
			logger.Error().Err(err).Msg("serverSendRequest")
			return err
		}
	}
	return nil
}

func (s *DeviceSession) deviceSendRespone(header *dp.Header) error {
	value, ok := s.sendIDStore.Get(timekv.Key(header.ID))

	if !ok {
		logger.Warn().Uint16("headerid", header.ID).Msg("deviceSendRespone")
		return ErrHeaderIDNotExist
	}
	bodyBuf := make([]byte, header.BodyLen)
	readLen, err := io.ReadFull(s.conn, bodyBuf)
	if err != nil {
		logger.Error().Int("readLen", readLen).Err(err).Msg("deviceSendRespone, ReadFull")
		return err
	}

	sendResp, err := dp.DecodeSendRespBody(header, bodyBuf)
	if err != nil {
		logger.Error().Err(err).Msg("deviceSendRespone")
		return err
	}
	value.C <- sendResp.Body
	return nil
}

func (s *DeviceSession) tcpIncomming(ctx context.Context, errChan chan<- error) {
	defer func() {
		logger.Debug().Msg("tcpIncomming exit")
	}()

	for {
		select {
		case <-ctx.Done():
			logger.Debug().Msg("tcpIncomming context done")
			return
		default:

			headerBuf := make([]byte, dp.HeaderLen)
			readLen, err := io.ReadFull(s.conn, headerBuf)
			logger.Debug().Int("readlen", readLen).Msg("tcpIncomming, read header")
			if err != nil {
				errChan <- err
				return
			}
			header, err := dp.DecodeHeader(headerBuf)
			if err != nil {
				errChan <- dp.ErrDecode
				return
			}
			logger.Debug().Uint16("headerid", header.ID).Str("type", header.Type.String()).Msg("tcpIncomming")
			switch header.Type {
			case dp.MsgType_DeviceAuthResp:
				if header.Code == dp.Code_Success {
					logger.Info().Msg("auth pass")
				} else {
					logger.Error().Str("resp.type", string(header.Code)).Msg("auth fail")
					errChan <- ErrAuthFailed
					return
				}
			case dp.MsgType_DevicePingResp:
			case dp.MsgType_ServerSendReq:
				err := s.serverSendRequest(header)
				if err != nil {
					logger.Error().Err(err).Msg("tcpIncomming")
					errChan <- err
					return
				}
			case dp.MsgType_DeviceSendResp:
				if err := s.deviceSendRespone(header); err != nil {
					errChan <- err
					return
				}
			default:
				errChan <- ErrDataType
				return
			}

		}
	}
}

func (s *DeviceSession) tcpOutgoing(ctx context.Context, errChan chan<- error) {
	defer func() {
		logger.Debug().Msg("tcpOutgoing exit")
	}()

	for {
		select {
		case <-ctx.Done():
			logger.Debug().Msg("tcpOutgoing context done")
			return
		case buf := <-s.outgoingChan:
			if n, err := ru.WriteFull(s.conn, buf); err != nil {
				logger.Error().Err(err).Int("writelen", n).Int("buflen", len(buf)).Msg("tcpOutgoing WriteFull error")
				errChan <- err
				return
			}
		}
	}
}

func (s *DeviceSession) Serve(ctx context.Context, errChan chan<- error) {

	devicelogger := logger.With().Str("device_ip", s.conn.LocalAddr().String()).Logger()
	devicelogger.Info().Str("deviceid", s.deviceID).Msg("serving")
	defer func() {
		devicelogger.Info().Msg("stop serving")
	}()

	deviceCtx, cancel := context.WithCancel(ctx)
	defer cancel()

	errNetChan := make(chan error, 2)
	go s.tcpIncomming(deviceCtx, errNetChan)
	go s.tcpOutgoing(deviceCtx, errNetChan)

	if err := s.auth(); err != nil {
		logger.Error().Err(err).Msg("send auth error")
		return
	}

	heartbeat := time.NewTicker(time.Second * 5)
	defer heartbeat.Stop()
	storeTicker := time.NewTicker(time.Second * 5)
	defer storeTicker.Stop()

	for {
		select {
		case err := <-errNetChan:
			errChan <- err
			logger.Error().Err(err).Msg("device done when error")
			cancel()
			return
		case <-deviceCtx.Done():
			logger.Debug().Msg("device done when deviceCtx done")
			if err := s.conn.Close(); err != nil {
				logger.Debug().Err(err).Msg("device close conn error")
			}
			return
		case <-heartbeat.C:
			// devicelogger.Debug().Msgf("heartbeat.C %s", time.Now().String())

		case <-storeTicker.C:
			// servelogger.Debug().Msgf("storeTicker.C %s", time.Now().String())
			s.sendIDStore.DelExpireKeys()
		}
	}
}

func (s *DeviceSession) SSFrames() <-chan *SSFrame {
	return s.ssframeChan
}

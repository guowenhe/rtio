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

package deviceproto

// REST-Like protocol

type Method uint8 // REST-Like Type

const (
	Method_Unknown         Method = 0
	Method_ConstrainedGet  Method = 1
	Method_ConstrainedPost Method = 2
	Method_ObservedGet     Method = 3
)

const (
	HeaderLen_RestMin         = 1
	HeaderLen_CoReq           = 5 // bytes
	HeaderLen_CoResp          = 1
	HeaderLen_ObGetEstabReq   = 7
	HeaderLen_ObGetEstabResp  = 3
	HeaderLen_ObGetNotifyReq  = 3
	HeaderLen_ObGetNotifyResp = 3
)

const (
	OBGET_OBSERVATIONS_MAX = 256
)

type StatusCode uint8

const (
	StatusCode_Unknown             StatusCode = 0
	StatusCode_InternalServerError StatusCode = 1
	StatusCode_OK                  StatusCode = 2
	StatusCode_Continue            StatusCode = 3
	StatusCode_Terminate           StatusCode = 4
	StatusCode_NotFount            StatusCode = 5
	StatusCode_BadRequest          StatusCode = 6
	StatusCode_MethodNotAllowed    StatusCode = 7
	StatusCode_TooManyRequests     StatusCode = 8
	StatusCode_TooManyObservations StatusCode = 9
)

func (c StatusCode) String() string {
	switch c {
	case StatusCode_Unknown:
		return "StatusCode_Unknown"
	case StatusCode_InternalServerError:
		return "StatusCode_InternalServerError"
	case StatusCode_OK:
		return "StatusCode_OK"
	case StatusCode_Continue:
		return "StatusCode_Continue"
	case StatusCode_Terminate:
		return "StatusCode_Terminate"
	case StatusCode_NotFount:
		return "StatusCode_NotFount"
	case StatusCode_BadRequest:
		return "StatusCode_BatRequest"
	case StatusCode_MethodNotAllowed:
		return "StatusCode_MethodNotAllowed"
	default:
	}
	return "Code_UndefineError"
}
func (c StatusCode) Error() string {
	return c.String()
}

// Using for CoGet and CoPost
type CoReq struct {
	HeaderID uint16 // redundant for low level message headerid
	Method   Method
	URI      uint32
	Data     []byte
}
type CoResp struct {
	HeaderID uint16 // redundant for low level message headerid
	Method   Method
	Code     StatusCode
	Data     []byte
}

type ObGetEstabReq struct {
	HeaderID uint16 // redundant for low level message headerid
	Method   Method
	ObID     uint16 // observation ID
	URI      uint32
	Data     []byte
}
type ObGetEstabResp struct {
	HeaderID uint16 // redundant for low level message headerid
	Method   Method
	Code     StatusCode
	ObID     uint16 // observation ID
}
type ObGetNotifyReq struct {
	HeaderID uint16 //  redundant for low level message headerid
	Method   Method
	Code     StatusCode
	ObID     uint16 // observation ID
	Data     []byte
}
type ObGetNotifyResp struct {
	HeaderID uint16 // redundant for low level message headerid
	Method   Method
	Code     StatusCode
	ObID     uint16 // observation ID
}

func DecodeMethod(buf []byte) (Method, error) {
	if len(buf) < HeaderLen_RestMin {
		return Method_Unknown, ErrNotEnought
	}
	return Method((buf[0] >> 4) & 0x0F), nil
}

func DecodeCoReq(headerID uint16, buf []byte) (*CoReq, error) {
	if len(buf) < HeaderLen_CoReq {
		return nil, ErrNotEnought
	}
	req := &CoReq{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		URI:      (uint32(buf[1]) << 24) + (uint32(buf[2]) << 16) + (uint32(buf[3]) << 8) + uint32(buf[4]),
		Data:     buf[5:],
	}
	return req, nil
}
func EncodeCoReq(req *CoReq, buf []byte) error {
	if len(buf) < HeaderLen_CoReq+len(req.Data) {
		return ErrNotEnought
	}
	buf[0] = (uint8(req.Method) << 4) & 0xF0
	buf[1] = uint8(req.URI>>24) & 0xFF
	buf[2] = uint8(req.URI>>16) & 0xFF
	buf[3] = uint8(req.URI>>8) & 0xFF
	buf[4] = uint8(req.URI) & 0xFF
	copy(buf[5:], req.Data)
	return nil
}
func EncodeCoReq_OverDeviceSendReq(req *CoReq, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_CoReq)+len(req.Data) {
		return ErrNotEnought
	}
	if err := EncodeCoReq(req, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_DeviceSendReq,
		ID:      req.HeaderID,
		BodyLen: HeaderLen_CoReq + uint16(len(req.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func EncodeCoReq_OverServerSendReq(req *CoReq, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_CoReq)+len(req.Data) {
		return ErrNotEnought
	}
	if err := EncodeCoReq(req, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_ServerSendReq,
		ID:      req.HeaderID,
		BodyLen: HeaderLen_CoReq + uint16(len(req.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func DecodeCoResp(headerID uint16, buf []byte) (*CoResp, error) {
	if len(buf) < HeaderLen_CoResp {
		return nil, ErrNotEnought
	}
	req := &CoResp{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		Code:     StatusCode(buf[0] & 0x0F),
		Data:     buf[1:],
	}
	return req, nil
}
func EncodeCoResp(resp *CoResp, buf []byte) error {
	if len(buf) < HeaderLen_CoResp+len(resp.Data) {
		return ErrNotEnought
	}
	buf[0] = ((uint8(resp.Method) << 4) & 0xF0) + (uint8(resp.Code) & 0x0F)
	copy(buf[1:], resp.Data)
	return nil
}

func EncodeCoResp_OverDeviceSendResp(resp *CoResp, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_CoResp)+len(resp.Data) {
		return ErrNotEnought
	}
	if err := EncodeCoResp(resp, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		ID:      resp.HeaderID,
		Version: Version,
		Type:    MsgType_DeviceSendResp,
		BodyLen: HeaderLen_CoResp + uint16(len(resp.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func EncodeCoResp_OverServerSendResp(resp *CoResp, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_CoResp)+len(resp.Data) {
		return ErrNotEnought
	}
	if err := EncodeCoResp(resp, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		ID:      resp.HeaderID,
		Version: Version,
		Type:    MsgType_ServerSendResp,
		BodyLen: HeaderLen_CoResp + uint16(len(resp.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}

func EncodeObGetEstabReq(req *ObGetEstabReq, buf []byte) error {
	if len(buf) < HeaderLen_ObGetEstabReq+len(req.Data) {
		return ErrNotEnought
	}
	buf[0] = (uint8(req.Method) << 4) & 0xF0
	buf[1] = uint8(req.ObID>>8) & 0xFF
	buf[2] = uint8(req.ObID) & 0xFF
	buf[3] = uint8(req.URI>>24) & 0xFF
	buf[4] = uint8(req.URI>>16) & 0xFF
	buf[5] = uint8(req.URI>>8) & 0xFF
	buf[6] = uint8(req.URI) & 0xFF
	copy(buf[7:], req.Data)
	return nil
}
func EncodeObGetEstabReq_OverServerSendReq(req *ObGetEstabReq, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_ObGetEstabReq)+len(req.Data) {
		return ErrNotEnought
	}
	if err := EncodeObGetEstabReq(req, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_ServerSendReq,
		ID:      req.HeaderID,
		BodyLen: HeaderLen_ObGetEstabReq + uint16(len(req.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func DecodeObGetEstabReq(headerID uint16, buf []byte) (*ObGetEstabReq, error) {
	if len(buf) < HeaderLen_ObGetEstabReq {
		return nil, ErrNotEnought
	}
	req := &ObGetEstabReq{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		ObID:     (uint16(buf[1]) << 8) + uint16(buf[2]),
		URI:      (uint32(buf[3]) << 24) + (uint32(buf[4]) << 16) + (uint32(buf[5]) << 8) + uint32(buf[6]),
		Data:     buf[7:],
	}
	return req, nil
}
func EncodeObGetEstabResp(resp *ObGetEstabResp, buf []byte) error {
	if len(buf) < HeaderLen_ObGetEstabResp {
		return ErrNotEnought
	}
	buf[0] = ((uint8(resp.Method) << 4) & 0xF0) + (uint8(resp.Code) & 0x0F)
	buf[1] = uint8(resp.ObID>>8) & 0xFF
	buf[2] = uint8(resp.ObID) & 0xFF
	return nil
}
func EncodeObGetEstabResp_OverServerSendResp(resp *ObGetEstabResp, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_ObGetEstabResp) {
		return ErrNotEnought
	}
	if err := EncodeObGetEstabResp(resp, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_ServerSendResp,
		ID:      resp.HeaderID,
		BodyLen: HeaderLen_ObGetEstabResp,
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}

func DecodeObGetEstabResp(headerID uint16, buf []byte) (*ObGetEstabResp, error) {
	if len(buf) < HeaderLen_ObGetEstabResp {
		return nil, ErrNotEnought
	}
	resp := &ObGetEstabResp{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		Code:     StatusCode(buf[0] & 0x0F),
		ObID:     (uint16(buf[1]) << 8) + uint16(buf[2]),
	}
	return resp, nil
}
func EncodeObGetNotifyReq(req *ObGetNotifyReq, buf []byte) error {
	if len(buf) < HeaderLen_ObGetNotifyReq+len(req.Data) {
		return ErrNotEnought
	}
	buf[0] = ((uint8(req.Method) << 4) & 0xF0) + (uint8(req.Code) & 0x0F)
	buf[1] = uint8(req.ObID>>8) & 0xFF
	buf[2] = uint8(req.ObID) & 0xFF

	copy(buf[3:], req.Data)
	return nil
}
func EncodeObGetNotifyReq_OverDeviceSendReq(req *ObGetNotifyReq, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_ObGetNotifyReq)+len(req.Data) {
		return ErrNotEnought
	}
	if err := EncodeObGetNotifyReq(req, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_DeviceSendReq,
		ID:      req.HeaderID,
		BodyLen: HeaderLen_ObGetNotifyReq + uint16(len(req.Data)),
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func DecodeObGetNotifyReq(headerID uint16, buf []byte) (*ObGetNotifyReq, error) {
	if len(buf) < HeaderLen_ObGetNotifyReq {
		return nil, ErrNotEnought
	}
	req := &ObGetNotifyReq{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		ObID:     (uint16(buf[1]) << 8) + uint16(buf[2]),
		Code:     StatusCode(buf[0] & 0x0F),
		Data:     buf[3:],
	}
	return req, nil
}
func EncodeObGetNotifyResp(resp *ObGetNotifyResp, buf []byte) error {
	if len(buf) < HeaderLen_ObGetNotifyResp {
		return ErrNotEnought
	}
	buf[0] = ((uint8(resp.Method) << 4) & 0xF0) + (uint8(resp.Code) & 0x0F)
	buf[1] = uint8(resp.ObID>>8) & 0xFF
	buf[2] = uint8(resp.ObID) & 0xFF
	return nil
}
func EncodeObGetNotifyResp_OverDeviceSendResp(resp *ObGetNotifyResp, buf []byte) error {
	if len(buf) < int(HeaderLen+HeaderLen_ObGetNotifyResp) {
		return ErrNotEnought
	}
	if err := EncodeObGetNotifyResp(resp, buf[HeaderLen:]); err != nil {
		return err
	}
	header := &Header{
		Version: Version,
		Type:    MsgType_DeviceSendResp,
		ID:      resp.HeaderID,
		BodyLen: HeaderLen_ObGetNotifyResp,
		Code:    Code_Success,
	}
	if err := EncodeHeader(header, buf[:HeaderLen]); err != nil {
		return err
	}
	return nil
}
func DecodeObGetNotifyResp(headerID uint16, buf []byte) (*ObGetNotifyResp, error) {
	if len(buf) < HeaderLen_ObGetNotifyResp {
		return nil, ErrNotEnought
	}
	resp := &ObGetNotifyResp{
		HeaderID: headerID,
		Method:   Method((buf[0] >> 4) & 0x0F),
		ObID:     (uint16(buf[1]) << 8) + uint16(buf[2]),
		Code:     StatusCode(buf[0] & 0x0F),
	}
	return resp, nil
}

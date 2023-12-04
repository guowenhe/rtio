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

import (
	"bytes"
	"errors"
)

type MsgType uint8

const (
	MsgType_UnknownType      MsgType = 0
	MsgType_DeviceVerifyReq  MsgType = 1
	MsgType_DeviceVerifyResp MsgType = 2
	MsgType_DevicePingReq    MsgType = 3
	MsgType_DevicePingResp   MsgType = 4
	MsgType_DeviceSendReq    MsgType = 5
	MsgType_DeviceSendResp   MsgType = 6
	MsgType_ServerSendReq    MsgType = 7
	MsgType_ServerSendResp   MsgType = 8
)

func (t MsgType) String() string {
	switch t {
	case MsgType_UnknownType:
		return "MsgType_UnknownType"
	case MsgType_DeviceVerifyReq:
		return "MsgType_DeviceVerifyReq"
	case MsgType_DeviceVerifyResp:
		return "MsgType_DeviceVerifyResp"
	case MsgType_DevicePingReq:
		return "MsgType_DevicePingReq"
	case MsgType_DevicePingResp:
		return "MsgType_DevicePingResp"
	case MsgType_DeviceSendReq:
		return "MsgType_DeviceSendReq"
	case MsgType_DeviceSendResp:
		return "MsgType_DeviceSendResp"
	case MsgType_ServerSendReq:
		return "MsgType_ServerSendReq"
	case MsgType_ServerSendResp:
		return "MsgType_ServerSendResp"
	default:
	}
	return "MsgType_UndefineError"
}

var (
	ErrUnkown       = errors.New("ErrUnkown")
	ErrExceedLength = errors.New("ErrExceedLength")
	ErrNotEnought   = errors.New("ErrNotEnought")
	ErrLengthError  = errors.New("ErrLengthError")
	ErrEncode       = errors.New("ErrEncode")
	ErrDecode       = errors.New("ErrDecode")
	ErrNetRead      = errors.New("ErrNetRead")
	ErrNetWrite     = errors.New("ErrNetWrite")
	ErrVerifyData   = errors.New("ErrVerifyData")
	ErrCapLevel     = errors.New("ErrCapLevel")
	ErrHeaderNil    = errors.New("ErrHeaderNil")
)

const (
	HeaderLen          uint16 = 5
	Version            uint8  = 0
	DeviceIDLen        uint16 = 36
	DeviceSecretLenMin uint16 = 24
	DeviceSecretLenMax uint16 = 64
)

var (
	capLevelToSize = []uint16{512, 1024, 2048, 4096}
)

// decode step：1.decode header first 2.decode body
// encode step：1.encode body first 2. encode header

type Header struct {
	Version uint8
	Type    MsgType
	ID      uint16
	BodyLen uint16
	Code    RemoteCode
}

type VerifydReq struct {
	Header       *Header
	CapLevel     uint8
	DeviceID     string
	DeviceSecret string
}

type VerifyResp struct {
	Header *Header
}
type PingReq struct {
	Header  *Header
	Timeout uint16
}
type PingResp struct {
	Header *Header
}
type SendReq struct {
	Header *Header
	Body   []byte
}
type SendResp struct {
	Header *Header
	Body   []byte
}

func GetCapSize(level uint8) (uint16, error) {

	if level > 3 {
		return 0, ErrCapLevel
	}
	return capLevelToSize[level], nil
}

func DecodeHeader(buf []byte) (*Header, error) {
	if len(buf) < int(HeaderLen) {
		return nil, ErrNotEnought
	}
	h := new(Header)
	h.Type = MsgType((buf[0] >> 4) & 0x0F)
	h.Version = (buf[0] >> 3) & 0x01
	h.Code = RemoteCode(buf[0] & 0x07)
	h.ID = (uint16(buf[1]) << 8) + uint16(buf[2])
	h.BodyLen = (uint16(buf[3]) << 8) + uint16(buf[4])
	if h.BodyLen > capLevelToSize[3] {
		return nil, ErrExceedLength
	}
	return h, nil
}
func EncodeHeader(h *Header, buf []byte) error {
	if len(buf) < int(HeaderLen) {
		return ErrNotEnought
	}
	buf[0] = ((uint8(h.Type) << 4) & 0xF0) + ((h.Version << 3) & 0x80) + (uint8(h.Code) & 0x07)
	buf[1] = uint8(h.ID>>8) & 0xFF      //msb
	buf[2] = uint8(h.ID) & 0xFF         //lsb
	buf[3] = uint8(h.BodyLen>>8) & 0xFF //msb
	buf[4] = uint8(h.BodyLen) & 0xFF    //lsb
	return nil
}

func DecodeVerifyReq(buf []byte) (*VerifydReq, error) {
	if len(buf) < int(HeaderLen+DeviceIDLen+DeviceSecretLenMin+1) {
		return nil, ErrNotEnought
	}
	if len(buf) > int(HeaderLen+1+DeviceIDLen+1+DeviceSecretLenMax) {
		return nil, ErrExceedLength
	}
	header, err := DecodeHeader(buf)
	if err != nil {
		return nil, err
	}
	req := new(VerifydReq)
	req.Header = header
	req.CapLevel = (buf[HeaderLen] >> 6) & 0x03
	bufVerify := buf[HeaderLen+1:]

	if p := bytes.Index(bufVerify, []byte(":")); p != -1 {
		deviceIDLen := p
		deviceSecretLen := len(bufVerify[p+1:])
		if deviceIDLen != int(DeviceIDLen) ||
			deviceSecretLen < int(DeviceSecretLenMin) ||
			deviceSecretLen > int(DeviceSecretLenMax) {
			return nil, ErrVerifyData
		}
		req.DeviceID = string(bufVerify[0:p])
		req.DeviceSecret = string(bufVerify[p+1:])
	} else {
		return nil, ErrVerifyData
	}
	return req, nil
}
func DecodeVerifyReqBody(header *Header, buf []byte) (*VerifydReq, error) {
	if nil == header {
		return nil, ErrHeaderNil
	}

	if len(buf) < int(DeviceIDLen+DeviceSecretLenMin+1) {
		return nil, ErrNotEnought
	}
	if len(buf) > int(1+DeviceIDLen+1+DeviceSecretLenMax) {
		return nil, ErrExceedLength
	}

	req := new(VerifydReq)
	req.Header = header
	req.CapLevel = (buf[0] >> 6) & 0x03
	bufVerify := buf[1:]

	if p := bytes.Index(bufVerify, []byte(":")); p != -1 {
		deviceIDLen := p
		deviceSecretLen := len(bufVerify[p+1:])
		if deviceIDLen != int(DeviceIDLen) ||
			deviceSecretLen < int(DeviceSecretLenMin) ||
			deviceSecretLen > int(DeviceSecretLenMax) {
			return nil, ErrVerifyData
		}
		req.DeviceID = string(bufVerify[0:p])
		req.DeviceSecret = string(bufVerify[p+1:])
	} else {
		return nil, ErrVerifyData
	}
	return req, nil
}
func EncodeVerifyReq(req *VerifydReq) ([]byte, error) {
	verifyData := req.DeviceID + ":" + req.DeviceSecret
	bodyLen := len(verifyData) + 1

	if bodyLen < int(1+DeviceIDLen+1+DeviceSecretLenMin) ||
		bodyLen > int(1+DeviceIDLen+1+DeviceSecretLenMax) {
		return nil, ErrVerifyData
	}
	buf := make([]byte, int(HeaderLen)+bodyLen)
	buf[HeaderLen] = (req.CapLevel << 6) & 0xc0
	copy(buf[HeaderLen+1:], verifyData)
	req.Header.BodyLen = uint16(bodyLen)
	if err := EncodeHeader(req.Header, buf); err != nil {
		return nil, err
	}
	return buf, nil
}
func DecodeVerifyResp(buf []byte) (*VerifyResp, error) {
	header, err := DecodeHeader(buf)
	if err != nil {
		return nil, err
	}
	resp := new(VerifyResp)
	resp.Header = header
	return resp, nil
}
func EncodeVerifyResp(resp *VerifyResp) ([]byte, error) {
	buf := make([]byte, int(HeaderLen))
	if err := EncodeHeader(resp.Header, buf); err != nil {
		return nil, err
	}
	return buf, nil
}

func DecodePingReqBody(header *Header, buf []byte) (*PingReq, error) {
	if nil == header {
		return nil, ErrHeaderNil
	}
	req := &PingReq{
		Header:  header,
		Timeout: 0,
	}
	if header.BodyLen == 0 {
		return req, nil
	} else if header.BodyLen == 2 {
		req.Timeout = (uint16(buf[0]) << 8) + uint16(buf[1])
	} else {
		return nil, ErrLengthError
	}
	return req, nil
}
func EncodePingReq(req *PingReq) ([]byte, error) {
	bodyLen := 0
	if req.Timeout != 0 {
		bodyLen = 2
	}
	buf := make([]byte, int(HeaderLen)+bodyLen)
	if bodyLen == 2 {
		buf[HeaderLen] = byte(req.Timeout >> 8)
		buf[HeaderLen+1] = byte(req.Timeout & 0xFF)
	}
	req.Header.BodyLen = uint16(bodyLen)
	if err := EncodeHeader(req.Header, buf); err != nil {
		return nil, err
	}
	return buf, nil
}
func EncodePingResp(resp *PingResp) ([]byte, error) {
	buf := make([]byte, int(HeaderLen))
	if err := EncodeHeader(resp.Header, buf); err != nil {
		return nil, err
	}
	return buf, nil
}

func DecodeSendReq(buf []byte) (*SendReq, error) {
	header, err := DecodeHeader(buf)
	if err != nil {
		return nil, err
	}
	req := new(SendReq)
	req.Header = header
	req.Body = buf[HeaderLen : HeaderLen+req.Header.BodyLen]
	return req, nil
}
func DecodeSendReqBody(header *Header, buf []byte) (*SendReq, error) {
	if nil == header {
		return nil, ErrHeaderNil
	}
	req := new(SendReq)
	req.Header = header
	req.Body = buf
	return req, nil
}
func EncodeSendReq(req *SendReq) ([]byte, error) {
	bodyLen := len(req.Body)
	req.Header.BodyLen = uint16(bodyLen)
	buf := make([]byte, int(HeaderLen)+bodyLen)
	if err := EncodeHeader(req.Header, buf); err != nil {
		return nil, err
	}
	copy(buf[HeaderLen:], req.Body)
	return buf, nil
}
func DecodeSendResp(buf []byte) (*SendResp, error) {
	header, err := DecodeHeader(buf)
	if err != nil {
		return nil, err
	}
	resp := new(SendResp)
	resp.Header = header
	resp.Body = buf[HeaderLen : HeaderLen+resp.Header.BodyLen]
	return resp, nil
}
func DecodeSendRespBody(header *Header, buf []byte) (*SendResp, error) {
	if nil == header {
		return nil, ErrHeaderNil
	}
	resp := new(SendResp)
	resp.Header = header
	resp.Body = buf
	return resp, nil
}
func EncodeSendResp(resp *SendResp) ([]byte, error) {
	bodyLen := len(resp.Body)
	resp.Header.BodyLen = uint16(bodyLen)
	buf := make([]byte, int(HeaderLen)+bodyLen)
	if err := EncodeHeader(resp.Header, buf); err != nil {
		return nil, err
	}
	copy(buf[HeaderLen:], resp.Body)
	return buf, nil
}

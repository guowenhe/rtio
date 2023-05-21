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
	"reflect"
	"testing"

	"gotest.tools/assert"
)

// func TestEncodeAuthReq(t *testing.T) {

// 	req := &AuthReq{
// 		CapLevel:     2,
// 		DeviceID:     "deviceID_001",
// 		DeviceSecret: "deviceSecret_001",
// 	}
// 	buf := make([]byte, 512)
// 	bodyLen, err := EncodeAuthReq(req, buf[HeaderLen:])
// 	if err != nil {
// 		t.Errorf("EncodeAuthReq() err != nil")
// 	}

// 	if bodyLen != 30 {
// 		t.Error("EncodeAuthReq bodyLen != 30")
// 	}

// 	req.Header = &Header{
// 		Version: Version,
// 		Type:    MsgType_DeviceAuthReq,
// 		ID:      0x8899,
// 		BodyLen: bodyLen,
// 		Code:    4,
// 	}
// 	err = EncodeHeader(req.Header, buf)
// 	if err != nil {
// 		t.Errorf("EncodeAuthReq() err != nil")
// 	}
// 	got := buf[0 : bodyLen+HeaderLen]
// 	want := make([]byte, bodyLen+HeaderLen)
// 	token := "deviceID_001" + ":" + "deviceSecret_001"
// 	copy(want[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x1e, 0x80})
// 	copy(want[HeaderLen+1:int(HeaderLen)+1+len(token)], token)
// 	if !bytes.Equal(buf[0:bodyLen+HeaderLen], want) {
// 		t.Errorf("Encode() = %q, want %q", got, want)
// 	}
// 	t.Logf("len=%d buf=%x", bodyLen, buf[0:40])
// }
// func TestDecodeAuthReq(t *testing.T) {

// 	token := "deviceID_001" + ":" + "deviceSecret_001"
// 	bodyLen := uint16(len(token)) + 1
// 	buf := make([]byte, HeaderLen+uint16(bodyLen))
// 	copy(buf[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x1e, 0x80})
// 	copy(buf[HeaderLen+1:int(HeaderLen)+1+len(token)], token)

// 	header, err := DecodeHeader(buf)
// 	if err != nil {
// 		t.Error("DecodeHeader(buf) err")
// 	}

// 	headerWant := &Header{
// 		Version: Version,
// 		Type:    MsgType_DeviceAuthReq,
// 		ID:      0x8899,
// 		BodyLen: bodyLen,
// 		Code:    4,
// 	}
// 	if !reflect.DeepEqual(header, headerWant) {
// 		t.Error("(heade != headerWant):", header, headerWant)
// 	}

// 	reqWant := &AuthReq{
// 		CapLevel:     2,
// 		DeviceID:     "deviceID_001",
// 		DeviceSecret: "deviceSecret_001",
// 	}
// 	req, err := DecodeAuthReq(buf[HeaderLen : header.BodyLen+HeaderLen])
// 	if err != nil {
// 		t.Error("DecodeHeader(buf) err")
// 	}

// 	if !reflect.DeepEqual(req, reqWant) {
// 		t.Error("(req != reqWant):", req, reqWant)
// 	}

// }
func TestEncodeAuthReq(t *testing.T) {
	// $ echo -n "cfa09baa-4913-4ad7-a936-2e26f9671b04:mb6bgso4EChvyzA05thF9+wH"| wc -L
	// 61
	// $ printf "%x\n" 61
	// 3d
	req := &AuthReq{
		Header: &Header{
			Version: Version,
			Type:    MsgType_DeviceAuthReq,
			ID:      0x8899,
			BodyLen: 0, // will update this field when decode
			Code:    4,
		},
		CapLevel:     1,
		DeviceID:     "cfa09baa-4913-4ad7-a936-2e26f9671b04",
		DeviceSecret: "mb6bgso4EChvyzA05thF9+wH",
	}

	buf, err := EncodeAuthReq(req)
	assert.NilError(t, err)
	assert.Equal(t, len(buf), int(HeaderLen+1+0x3d))

	want := make([]byte, HeaderLen+1+0x3d)
	token := "cfa09baa-4913-4ad7-a936-2e26f9671b04" + ":" + "mb6bgso4EChvyzA05thF9+wH"
	copy(want[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x3d + 1, 0x40})
	copy(want[HeaderLen+1:int(HeaderLen)+1+len(token)], token)
	if !bytes.Equal(buf, want) {
		t.Errorf("Encode() = %q, want %q", buf, want)
	}
	t.Logf("len=%d buf=%x", len(buf), buf)
}

func TestDecodeAuthReq(t *testing.T) {

	token := "cfa09baa-4913-4ad7-a936-2e26f9671b04" + ":" + "mb6bgso4EChvyzA05thF9+wH"
	bodyLen := uint16(len(token)) + 1
	buf := make([]byte, HeaderLen+uint16(bodyLen))
	copy(buf[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x3d + 1, 0x40})
	copy(buf[HeaderLen+1:int(HeaderLen)+1+len(token)], token)

	headerWant := &Header{
		Version: Version,
		Type:    MsgType_DeviceAuthReq,
		ID:      0x8899,
		BodyLen: bodyLen,
		Code:    4,
	}
	reqWant := &AuthReq{
		Header:       headerWant,
		CapLevel:     1,
		DeviceID:     "cfa09baa-4913-4ad7-a936-2e26f9671b04",
		DeviceSecret: "mb6bgso4EChvyzA05thF9+wH",
	}
	req, err := DecodeAuthReq(buf)

	if err != nil {
		t.Error("DecodeAuthReq err", err)
	}

	if !reflect.DeepEqual(req, reqWant) {
		t.Error("(req != reqWant):", req, reqWant)
	}
	t.Logf("req=%v", req)

}
func TestDecodeAuthReqDeviceID(t *testing.T) {
	token := "cfa09baa-4913-4ad7-a936-2e26f9671b04f" + ":" + "mb6bgso4EChvyzA05thF9+wH"
	bodyLen := uint16(len(token)) + 1
	buf := make([]byte, HeaderLen+uint16(bodyLen))
	copy(buf[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x3e + 1, 0x40})
	copy(buf[HeaderLen+1:int(HeaderLen)+1+len(token)], token)
	req, err := DecodeAuthReq(buf)
	assert.Equal(t, err, ErrAuthData)
	t.Logf("req=%v", req)
}
func TestDecodeAuthReqDeviceSecret(t *testing.T) {
	{
		token := "cfa09baa-4913-4ad7-a936-2e26f9671b04" + ":" + "mb6bgso4EChvyzA05thF9+w"
		bodyLen := uint16(len(token)) + 1
		buf := make([]byte, HeaderLen+uint16(bodyLen))
		copy(buf[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x3c + 1, 0x40})
		copy(buf[HeaderLen+1:int(HeaderLen)+1+len(token)], token)
		req, err := DecodeAuthReq(buf)
		assert.Equal(t, err, ErrAuthData)
		t.Logf("req=%v", req)
	}
	{
		token := "cfa09baa-4913-4ad7-a936-2e26f9671b04" + ":" + "12345678901234567890123456789012345678901234567890123456789012345"
		bodyLen := uint16(len(token)) + 1
		buf := make([]byte, HeaderLen+uint16(bodyLen))
		copy(buf[0:HeaderLen+1], []byte{0x14, 0x88, 0x99, 0x00, 0x3c + 1, 0x40})
		copy(buf[HeaderLen+1:int(HeaderLen)+1+len(token)], token)
		req, err := DecodeAuthReq(buf)
		assert.Equal(t, err, ErrExceedLength)
		t.Logf("req=%v", req)
	}

}
func TestEncodeSendReq(t *testing.T) {
	body := []byte{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}
	req := &SendReq{
		Header: &Header{
			Version: Version,
			Type:    MsgType_ServerSendReq,
			ID:      0x8899,
			BodyLen: 0, // will update this field when encode
			Code:    4,
		},
		Body: body,
	}

	buf, err := EncodeSendReq(req)
	assert.NilError(t, err)
	assert.Equal(t, len(buf), int(int(HeaderLen)+len(body)))
	want := make([]byte, int(HeaderLen)+len(body))
	copy(want[0:HeaderLen], []byte{0x74, 0x88, 0x99, 0x00, uint8(len(body))})
	copy(want[HeaderLen:int(HeaderLen)+len(body)], body)
	if !bytes.Equal(buf, want) {
		t.Errorf("Encode() = %q, want %q", buf, want)
	}
	t.Logf("len=%d buf=%x", len(buf), buf)
}
func TestDecodeSendReq(t *testing.T) {
	body := []byte{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}
	buf := make([]byte, int(HeaderLen)+len(body))
	copy(buf[0:HeaderLen], []byte{0x74, 0x88, 0x99, 0x00, uint8(len(body))})
	copy(buf[HeaderLen:int(HeaderLen)+len(body)], body)

	req, err := DecodeSendReq(buf)
	assert.NilError(t, err)

	reqWant := &SendReq{
		Header: &Header{
			Version: Version,
			Type:    MsgType_ServerSendReq,
			ID:      0x8899,
			BodyLen: uint16(len(body)),
			Code:    4,
		},
		Body: body,
	}
	if !reflect.DeepEqual(req, reqWant) {
		t.Error("(req != reqWant):", req, reqWant)
	}
	t.Logf("req=%v", req)

}

func TestEncodeSendResp(t *testing.T) {
	body := []byte{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}
	req := &SendResp{
		Header: &Header{
			Version: Version,
			Type:    MsgType_ServerSendResp,
			ID:      0x8899,
			BodyLen: 0, // will update this field when decode
			Code:    4,
		},
		Body: body,
	}

	buf, err := EncodeSendResp(req)
	assert.NilError(t, err)
	assert.Equal(t, len(buf), int(int(HeaderLen)+len(body)))
	want := make([]byte, int(HeaderLen)+len(body))
	copy(want[0:HeaderLen], []byte{0x84, 0x88, 0x99, 0x00, uint8(len(body))})
	copy(want[HeaderLen:int(HeaderLen)+len(body)], body)
	if !bytes.Equal(buf, want) {
		t.Errorf("Encode() = %q, want %q", buf, want)
	}
	t.Logf("len=%d buf=%x", len(buf), buf)
}
func TestDecodeSendResp(t *testing.T) {
	body := []byte{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a}
	buf := make([]byte, int(HeaderLen)+len(body))
	copy(buf[0:HeaderLen], []byte{0x84, 0x88, 0x99, 0x00, uint8(len(body))})
	copy(buf[HeaderLen:int(HeaderLen)+len(body)], body)

	req, err := DecodeSendResp(buf)
	assert.NilError(t, err)

	reqWant := &SendResp{
		Header: &Header{
			Version: Version,
			Type:    MsgType_ServerSendResp,
			ID:      0x8899,
			BodyLen: uint16(len(body)),
			Code:    4,
		},
		Body: body,
	}
	if !reflect.DeepEqual(req, reqWant) {
		t.Error("(req != reqWant):", req, reqWant)
	}
	t.Logf("req=%v", req)

}

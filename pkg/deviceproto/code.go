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

type RemoteCode uint8

const (
	Code_UnkownErr   = RemoteCode(0)
	Code_Success     = RemoteCode(0x01)
	Code_TypeErr     = RemoteCode(0x02)
	Code_AuthFail    = RemoteCode(0x03)
	Code_ParaInvalid = RemoteCode(0x04)
	Code_LengthErr   = RemoteCode(0x05)
	Code_ResNotFound = RemoteCode(0x06)
)

func (c RemoteCode) String() string {
	switch c {
	case Code_UnkownErr:
		return "Code_UnkownErr"
	case Code_Success:
		return "Code_Success"
	case Code_TypeErr:
		return "Code_TypeErr"
	case Code_AuthFail:
		return "Code_AuthFail"
	case Code_ParaInvalid:
		return "Code_ParaInvalid"
	case Code_LengthErr:
		return "Code_LengthErr"
	case Code_ResNotFound:
		return "Code_ResNotFound"
	default:
	}
	return "Code_UndefineError"
}
func (c RemoteCode) Error() string {
	return c.String()
}

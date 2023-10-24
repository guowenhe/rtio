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

package rtioutils

import (
	"crypto/rand"
	"hash/crc32"
	"math/big"
	"net"
)

func GenUint16ID() (uint16, error) {
	r, e := rand.Int(rand.Reader, big.NewInt(65536))
	return uint16(r.Uint64()), e
}
func GenUint32ID() (uint32, error) {
	r, e := rand.Int(rand.Reader, big.NewInt(4294967296))
	return uint32(r.Uint64()), e
}

func WriteFull(w net.Conn, buf []byte) (n int, err error) {
	writelen := 0
	buflen := len(buf)
	for writelen < buflen {
		n, err := w.Write(buf[writelen:buflen])
		writelen += n
		if err != nil {
			return writelen, err
		}
	}
	return writelen, err
}

func URIHash(uri string) uint32 {
	return crc32.ChecksumIEEE([]byte(uri))
}

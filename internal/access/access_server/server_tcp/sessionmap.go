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

package server_tcp

import (
	"sync"
)

type SessionMap struct {
	store sync.Map
}

func (s *SessionMap) Set(deviceID string, value *Session) {
	logger.Debug().Str("deviceid", deviceID).Msg("Set")
	s.store.Store(deviceID, value)
}
func (s *SessionMap) Get(deviceID string) (*Session, bool) {
	v, ok := s.store.Load(deviceID)
	logger.Debug().Str("deviceid", deviceID).Bool("ok", ok).Msg("Get")
	if !ok {
		return nil, false
	}
	return v.(*Session), ok
}
func (s *SessionMap) Del(deviceID string) {
	logger.Debug().Str("deviceid", deviceID).Msg("Del")
	s.store.Delete(deviceID)
}

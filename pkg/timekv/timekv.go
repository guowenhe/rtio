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

package timekv

import (
	"sync"
	"time"

	"github.com/rs/zerolog/log"
)

// base on sync.Map, could set concurrency
// could modiy Value and key, for a new timekv lib
type Key uint16

type Value struct {
	internelID int // for unit test
	C          chan []byte
}

type _Store struct {
	storeTime time.Time
	value     *Value
}
type TimeKV struct {
	store  sync.Map
	expire time.Duration
}

func NewTimeKV(expire time.Duration) *TimeKV {
	t := &TimeKV{}
	t.expire = expire
	return t
}

func (s *TimeKV) DelExpireKeys() {
	f := func(k, v interface{}) bool {
		if time.Since(v.(*_Store).storeTime) > s.expire {
			log.Warn().Interface("key", k).Msg("expired and deleted")
			s.store.Delete(k)
		}
		return true
	}
	s.store.Range(f)
}

func (s *TimeKV) Del(key Key) {
	s.store.Delete(key)
}

func (s *TimeKV) Get(key Key) (*Value, bool) {
	v, ok := s.store.Load(key)
	if !ok {
		return nil, false
	}
	return v.(*_Store).value, ok
}

func (s *TimeKV) Set(key Key, value *Value) {
	s.store.Store(key, &_Store{
		storeTime: time.Now(),
		value:     value,
	})
}

func (s *TimeKV) RangeKV(f func(Key, *Value) bool) {
	s.store.Range(func(key, value any) bool {
		return f(key.(Key), value.(*_Store).value)
	})
}

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
	"net"
	"sync"
	"sync/atomic"
	"time"

	"github.com/rs/zerolog/log"
)

type ServerTCP struct {
	listener   net.Listener
	sessions   *SessionMap
	wait       *sync.WaitGroup
	sessionNum int32
}

func NewServerTCP(addr string, sessionMap *SessionMap) (*ServerTCP, error) {

	listener, err := net.Listen("tcp", addr)
	if err != nil {
		log.Error().Err(err).Msg("listen failed")
		return nil, err
	}

	return &ServerTCP{
		listener:   listener,
		sessions:   sessionMap,
		wait:       &sync.WaitGroup{},
		sessionNum: 0,
	}, nil
}

func (s *ServerTCP) AddSession(deviceID string, session *Session) {
	invalid, ok := s.sessions.Get(deviceID)
	if ok {
		invalid.Cancel()
		log.Debug().Msg("invalid session cancel")
		<-invalid.Done()
		log.Debug().Msg("invalid session done v2.01")
	}
	atomic.AddInt32(&s.sessionNum, 1)
	s.sessions.Set(deviceID, session)

}
func (s *ServerTCP) DelSession(deviceID string) {
	s.sessions.Del(deviceID)
	atomic.AddInt32(&s.sessionNum, -1)
}

func (s *ServerTCP) Shutdown() {
	log.Info().Msg("shutdown")
	if s.listener != nil {
		s.listener.Close()
	}
}

func (s *ServerTCP) Serve(c context.Context) {
	ctx, cancel := context.WithCancel(c)
	log.Info().Str("addr", s.listener.Addr().String()).Msg("server started")

	go func() {
		t := time.NewTicker(time.Second * 10)
	EXIT_LOOPY:
		for {
			select {
			case <-ctx.Done():
				log.Debug().Msg("context done")
				break EXIT_LOOPY
			case <-t.C:
				// log.Error().Int32("sessionnum", s.sessionNum).Msg("") // for stress
			}
		}
		s.Shutdown()
	}()

	s.wait.Add(1)
	go func() {
		defer s.wait.Done()
		defer cancel()
		for s.listener != nil {
			conn, err := s.listener.Accept()
			if err != nil {
				if errors.Is(err, net.ErrClosed) {
					log.Warn().Msg("listener error closed")
				} else {
					log.Error().Err(err).Msg("listener accept error")
				}
				break
			}
			s.wait.Add(1)
			session := newSession(conn)
			go session.serve(ctx, s.wait, s.AddSession, s.DelSession)
		}
		log.Info().Msg("listener closed")
	}()

	log.Info().Msg("waiting for subroutes")
	if s.wait != nil {
		s.wait.Wait()
	}
	log.Info().Msg("waiting end")
}

func InitTCPServer(ctx context.Context, addr string,
	sessionMap *SessionMap, wait *sync.WaitGroup) error {

	s, err := NewServerTCP(addr, sessionMap)
	if err != nil {
		log.Error().Err(err).Msg("NewServerTCP error")
		return err
	}
	wait.Add(1)
	go func() {
		defer wait.Done()
		s.Serve(ctx)
	}()
	return nil
}

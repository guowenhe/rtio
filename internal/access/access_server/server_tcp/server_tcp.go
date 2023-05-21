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
	"context"
	"errors"
	"net"
	"rtio2/pkg/log"
	"sync"
)

type ServerTCP struct {
	listener net.Listener
	sessions *SessionMap
	wait     *sync.WaitGroup
}

var logger log.Logger

func init() {
	logger = log.With().Str("module", "server_tcp").Logger()
}

func NewServerTCP(addr string) (*ServerTCP, error) {

	listener, err := net.Listen("tcp", addr)
	if err != nil {
		log.Error().Err(err).Msg("listen failed")
		return nil, err
	}

	return &ServerTCP{
		listener: listener,
		sessions: &SessionMap{},
		wait:     &sync.WaitGroup{},
	}, nil
}

func (s *ServerTCP) Start(c context.Context) {
	ctx, cancel := context.WithCancel(c)
	logger.Info().Str("addr", s.listener.Addr().String()).Msg("server started")

	go func() {
		<-ctx.Done()
		logger.Debug().Msg("context done")
		s.Stop()
	}()

	s.wait.Add(1)
	go func() {
		defer s.wait.Done()
		defer cancel()
		for s.listener != nil {
			conn, err := s.listener.Accept()
			if err != nil {
				if errors.Is(err, net.ErrClosed) {
					logger.Warn().Msg("listener error closed")
				} else {
					logger.Error().Err(err).Msg("listener accept error")
				}
				break
			}
			s.wait.Add(1)
			session := newSession(conn)
			go session.serve(ctx, s.wait, s.AddSession, s.DelSession)
		}
		logger.Info().Msg("listener closed")
	}()

}
func (s *ServerTCP) AddSession(deviceID string, session *Session) {
	invalid, ok := s.sessions.Get(deviceID)
	if ok {
		invalid.Cancel()
		logger.Debug().Msg("invalid session cancel")
		<-invalid.Done()
		logger.Debug().Msg("invalid session done")
	}
	s.sessions.Set(deviceID, session)
}
func (s *ServerTCP) DelSession(deviceID string) {
	s.sessions.Del(deviceID)
}

func (s *ServerTCP) Stop() {
	logger.Info().Msg("stopping and cleaning up")
	if s.listener != nil {
		s.listener.Close()
	}
}

func (s *ServerTCP) Wait() {
	logger.Info().Msg("waiting for subroutes")
	if s.wait != nil {
		s.wait.Wait()
	}
	logger.Info().Msg("waiting end")
}

func TCPServe(ctx context.Context, addr string, sessionMapChan chan<- *SessionMap, errChan chan<- error, wait *sync.WaitGroup) {
	defer wait.Done()

	s, err := NewServerTCP(addr)
	if err != nil {
		logger.Fatal().Err(err).Msg("NewServerTCP error")
		errChan <- err
		return
	}
	sessionMapChan <- s.sessions
	s.Start(ctx)
	s.Wait()
	logger.Info().Msg("serve stoped")
}

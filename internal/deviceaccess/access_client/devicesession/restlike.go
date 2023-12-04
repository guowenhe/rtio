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
	dp "rtio2/pkg/deviceproto"
	"time"

	"github.com/rs/zerolog/log"
)

var (
	ErrAlreadyRegistered = errors.New("ErrAlreadyRegistered")
)

// not thread-safe
func (s *DeviceSession) RegisterGetHandler(uri uint32, handler func(req []byte) ([]byte, error)) error {
	_, ok := s.regGetHandlerMap[uri]
	if ok {
		return ErrAlreadyRegistered
	}
	s.regGetHandlerMap[uri] = handler

	return nil
}

// not thread-safe
func (s *DeviceSession) RegisterPostHandler(uri uint32, handler func(req []byte) ([]byte, error)) error {
	_, ok := s.regPostHandlerMap[uri]
	if ok {
		return ErrAlreadyRegistered
	}
	s.regPostHandlerMap[uri] = handler
	return nil
}

// not thread-safe
func (s *DeviceSession) RegisterObGetHandler(uri uint32, handler func(ctx context.Context, req []byte) (<-chan []byte, error)) error {
	_, ok := s.regObGetHandlerMap[uri]
	if ok {
		return ErrAlreadyRegistered
	}

	s.regObGetHandlerMap[uri] = handler
	return nil
}

var (
	ErrInternel            = errors.New("ErrInternel")
	ErrInternalServerError = errors.New("ErrInternalServerError")
	ErrResourceNotFount    = errors.New("ErrResourceNotFount")
	ErrBadRequest          = errors.New("ErrBadRequest")
	ErrMethodNotAllowed    = errors.New("ErrMethodNotAllowed")
	ErrTooManyRequests     = errors.New("ErrTooManyRequests")
	ErrRequestTimeout      = errors.New("ErrRequestTimeout")
)

func transToSDKError(code dp.StatusCode) error {
	switch code {
	case dp.StatusCode_Unknown:
		return ErrInternalServerError
	case dp.StatusCode_InternalServerError:
		return ErrInternalServerError
	case dp.StatusCode_OK:
		return nil
	case dp.StatusCode_Continue:
		return ErrInternalServerError
	case dp.StatusCode_Terminate:
		return ErrInternalServerError
	case dp.StatusCode_NotFount:
		return ErrResourceNotFount
	case dp.StatusCode_BadRequest:
		return ErrBadRequest
	case dp.StatusCode_MethodNotAllowed:
		return ErrMethodNotAllowed
	case dp.StatusCode_TooManyRequests:
		return ErrTooManyRequests
	case dp.StatusCode_TooManyObservations:
		return ErrInternalServerError
	default:
		return ErrInternel
	}
}

func (s *DeviceSession) Get(uri uint32, Req []byte, timeout time.Duration) ([]byte, error) {
	headerID := s.genHeaderID()
	respChan, err := s.sendCoReq(headerID, dp.Method_ConstrainedGet, uri, Req)
	if err != nil {
		log.Error().Err(err).Msg("Get")
		return nil, ErrInternel
	}
	statusCode, data, err := s.receiveCoResp(headerID, respChan, timeout)
	if err != nil {
		if err == ErrSendTimeout {
			log.Error().Err(err).Msg("Get")
			return nil, ErrRequestTimeout
		}
		log.Error().Err(err).Msg("Get")
		return nil, ErrInternel
	}
	return data, transToSDKError(statusCode)
}
func (s *DeviceSession) Get2(ctx context.Context, uri uint32, Req []byte, timeout time.Duration) ([]byte, error) {
	headerID := s.genHeaderID()
	respChan, err := s.sendCoReq(headerID, dp.Method_ConstrainedGet, uri, Req)
	if err != nil {
		log.Error().Err(err).Msg("Get")
		return nil, ErrInternel
	}
	statusCode, data, err := s.receiveCoRespWithContext(ctx, headerID, respChan)
	if err != nil {
		if err == ErrSendTimeout {
			log.Error().Err(err).Msg("Get")
			return nil, ErrRequestTimeout
		}
		log.Error().Err(err).Msg("Get")
		return nil, ErrInternel
	}
	return data, transToSDKError(statusCode)
}

func (s *DeviceSession) Post(uri uint32, Req []byte, timeout time.Duration) ([]byte, error) {
	headerID := s.genHeaderID()
	respChan, err := s.sendCoReq(headerID, dp.Method_ConstrainedPost, uri, Req)
	if err != nil {
		log.Error().Err(err).Msg("Post")
		return nil, ErrInternel
	}
	statusCode, data, err := s.receiveCoResp(headerID, respChan, timeout)
	if err != nil {
		if err == ErrSendTimeout {
			log.Error().Err(err).Msg("Post")
			return nil, ErrRequestTimeout
		}
		log.Error().Err(err).Msg("Post")
		return nil, ErrInternel
	}
	return data, transToSDKError(statusCode)
}

func (s *DeviceSession) Post2(ctx context.Context, uri uint32, Req []byte, timeout time.Duration) ([]byte, error) {
	headerID := s.genHeaderID()
	respChan, err := s.sendCoReq(headerID, dp.Method_ConstrainedPost, uri, Req)
	if err != nil {
		log.Error().Err(err).Msg("Post")
		return nil, ErrInternel
	}
	statusCode, data, err := s.receiveCoRespWithContext(ctx, headerID, respChan)
	if err != nil {
		if err == ErrSendTimeout {
			log.Error().Err(err).Msg("Post")
			return nil, ErrRequestTimeout
		}
		log.Error().Err(err).Msg("Post")
		return nil, ErrInternel
	}
	return data, transToSDKError(statusCode)
}

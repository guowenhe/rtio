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

package main

import (
	"context"
	"flag"
	"fmt"
	"net"

	"rtio2/pkg/config"
	"rtio2/pkg/logsettings"
	"rtio2/pkg/rpcproto/deviceservice"
	"rtio2/pkg/rtioutils"

	"github.com/rs/zerolog/log"
	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 17912, "The server port")
)

type server struct {
	deviceservice.UnimplementedDeviceServiceServer
}

func (s *server) Get(ctx context.Context, req *deviceservice.Req) (*deviceservice.Resp, error) {
	log.Info().Uint32("uri", req.Uri).Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Msg("Get")

	resp := &deviceservice.Resp{
		Id:   req.Id,
		Code: deviceservice.Code_CODE_METHOD_NOT_ALLOWED,
	}

	switch req.Uri {
	case rtioutils.URIHash("/uri/example1"): //0x7747f5cc
		log.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Get")
		resp.Code = deviceservice.Code_CODE_OK
		resp.Data = []byte("resp with data, with get /uri/example1")

	case rtioutils.URIHash("/uri/example2"): //0xee4ea476
		log.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Get")
		resp.Code = deviceservice.Code_CODE_OK
		resp.Data = []byte("resp with data, with get /uri/example2")
	}

	return resp, nil
}
func (s *server) Post(ctx context.Context, req *deviceservice.Req) (*deviceservice.Resp, error) {
	log.Info().Uint32("id", req.Id).Uint32("uri", req.Uri).Str("DeviceID", req.DeviceId).Msg("Post")

	resp := &deviceservice.Resp{
		Id:   req.Id,
		Code: deviceservice.Code_CODE_METHOD_NOT_ALLOWED,
	}

	switch req.Uri {
	case rtioutils.URIHash("/uri/example1"): //0x7747f5cc
		log.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Post")
		resp.Code = deviceservice.Code_CODE_OK
		resp.Data = []byte("resp with data, with post /uri/example1")

	case rtioutils.URIHash("/uri/example2"): //0xee4ea476
		log.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Post")
		resp.Code = deviceservice.Code_CODE_OK
		resp.Data = []byte("resp with data, with post /uri/example2")
	}

	return resp, nil
}

func main() {

	config.StringKV.Set("log.format", "text")
	config.StringKV.Set("log.level", "debug")
	logsettings.Set()
	log.Info().Msg("server starting ...")

	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatal().Err(err).Msg("failed to listen")
	}
	s := grpc.NewServer()

	deviceservice.RegisterDeviceServiceServer(s, &server{})
	log.Info().Str("addr", lis.Addr().String()).Msg("server started.")
	if err := s.Serve(lis); err != nil {
		log.Fatal().Err(err).Msg("failed to serve")
	}
}

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
	"rtio2/pkg/log"
	"rtio2/pkg/rpcproto/resource"

	"google.golang.org/grpc"
)

var (
	logger log.Logger
	port   = flag.Int("port", 17912, "The server port")
)

type server struct {
	resource.UnimplementedResourceServiceServer
}

func (s *server) Get(ctx context.Context, req *resource.Req) (*resource.Resp, error) {
	logger.Info().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Msg("Get")

	logger.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Get")
	resp := &resource.Resp{
		Id:   req.Id,
		Code: resource.Code_CODE_OK,
		Data: []byte("resp with data"),
	}

	return resp, nil
}
func (s *server) Post(ctx context.Context, req *resource.Req) (*resource.Resp, error) {
	logger.Info().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Msg("Post")

	logger.Debug().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("req", string(req.Data)).Msg("Post")

	// resp := &resource.Resp{
	// 	Id:   req.Id,
	// 	Code: resource.Code_CODE_METHOD_NOT_ALLOWED,
	// }
	resp := &resource.Resp{
		Id:   req.Id,
		Code: resource.Code_CODE_OK,
		Data: []byte("resp with data, with post"),
	}
	return resp, nil
}

func main() {
	log.OpenFile("resource.log")
	defer log.CloseFile()

	logger = log.With().Str("module", "resource").Logger()
	logger.Info().Msg("server starting ...")

	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		logger.Fatal().Err(err).Msg("failed to listen")
	}
	s := grpc.NewServer()

	resource.RegisterResourceServiceServer(s, &server{})
	logger.Info().Str("addr", lis.Addr().String()).Msg("server started.")
	if err := s.Serve(lis); err != nil {
		logger.Fatal().Err(err).Msg("failed to serve")
	}
}

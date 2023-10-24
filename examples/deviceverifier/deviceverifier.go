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
	"rtio2/pkg/rpcproto/deviceverifier"

	"github.com/rs/zerolog/log"
	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 17915, "The server port")
)

type server struct {
	deviceverifier.UnimplementedVerifierServiceServer
}

func (s *server) Verify(ctx context.Context, req *deviceverifier.VerifyReq) (*deviceverifier.VerifyResp, error) {
	log.Info().Uint32("id", req.Id).Str("DeviceID", req.DeviceId).Str("DeviceSecret", string(req.DeviceSecret)[len(req.DeviceSecret)-6:]).Msg("Verifier")
	resp := &deviceverifier.VerifyResp{
		Id:   req.Id,
		Code: deviceverifier.Code_CODE_PASS,
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

	deviceverifier.RegisterVerifierServiceServer(s, &server{})
	log.Info().Str("addr", lis.Addr().String()).Msg("server started.")
	if err := s.Serve(lis); err != nil {
		log.Fatal().Err(err).Msg("failed to serve")
	}
}

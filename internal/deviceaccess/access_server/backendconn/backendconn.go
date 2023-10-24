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

package backendconn

import (
	"rtio2/pkg/config"
	"rtio2/pkg/rpcproto/deviceservice"
	"rtio2/pkg/rpcproto/deviceverifier"

	"github.com/rs/zerolog/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

var (
	serviceAddr string
	serviceConn *grpc.ClientConn
)

var (
	verifierAddr string
	verifierConn *grpc.ClientConn
)

func dial(addr string) (*grpc.ClientConn, error) {
	conn, err := grpc.Dial(
		addr,
		grpc.WithDefaultServiceConfig(`{"loadBalancingConfig": [{"round_robin":{}}]}`),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		log.Error().Err(err).Str("addr", addr).Msg("dial error")
		return nil, err
	}
	return conn, nil

}

func GetDeviceServiceClient() (deviceservice.DeviceServiceClient, error) {

	if serviceConn != nil {
		return deviceservice.NewDeviceServiceClient(serviceConn), nil
	}
	var err error
	serviceConn, err = dial(serviceAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", serviceAddr).Msg("device service reconnect failed")
		return nil, err
	} else {
		return deviceservice.NewDeviceServiceClient(serviceConn), nil
	}
}

func GetDeviceVerifierClient() (deviceverifier.VerifierServiceClient, error) {

	if verifierConn != nil {
		return deviceverifier.NewVerifierServiceClient(verifierConn), nil
	}
	var err error
	verifierConn, err = dial(verifierAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", verifierAddr).Msg("deviceVerifier service reconnect failed")
		return nil, err
	} else {
		return deviceverifier.NewVerifierServiceClient(verifierConn), nil
	}
}

func InitBackendConnn() {
	var err error
	serviceAddr = config.StringKV.GetWithDefault("backend.deviceservice", "deviceservice.rtio:17912")
	serviceConn, err = dial(serviceAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", serviceAddr).Msg("device service conn failed")
	}

	verifierAddr = config.StringKV.GetWithDefault("backend.deviceverifier", "deviceverifier.rtio:17915")
	verifierConn, err = dial(verifierAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", verifierAddr).Msg("deviceVerifier service conn failed")
	}
}

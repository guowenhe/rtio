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
	"errors"
	"os"

	"rtio2/pkg/rpcproto/resource"

	"github.com/rs/zerolog/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

var (
	ErrResourceNotFound = errors.New("ErrResourceNotFound")
	resourceServiceAddr string
	resourceServiceConn *grpc.ClientConn
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

func GetResourceServiceClient() (resource.ResourceServiceClient, error) {

	if resourceServiceConn != nil {
		return resource.NewResourceServiceClient(resourceServiceConn), nil
	}
	var err error
	resourceServiceConn, err = dial(resourceServiceAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", resourceServiceAddr).Msg("resource service reconnect failed")
		return nil, err
	} else {
		return resource.NewResourceServiceClient(resourceServiceConn), nil
	}
}

func InitBackendConnn() {

	resourceServiceAddr = os.Getenv("RTIO_RESOURCE_SERVICE_ADDR") // example "127.0.0.1:17912"

	if len(resourceServiceAddr) == 0 {
		log.Info().Msg("using DNS [rtio-resource:17912] connect to rtio-resource service")
		resourceServiceAddr = "rtio-resource:17912"
	}
	var err error
	resourceServiceConn, err = dial(resourceServiceAddr)
	if err != nil {
		log.Error().Err(err).Str("addr", resourceServiceAddr).Msg("resource service conn failed")
	}
}

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

package router

import (
	"context"
	"errors"
	"hash/crc32"
	"rtio2/pkg/log"
	"sync"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

var logger log.Logger
var resorceMap map[uint32]*grpc.ClientConn

var (
	ErrResourceNotFound = errors.New("ErrResourceNotFound")
)

func Serve(ctx context.Context, wait *sync.WaitGroup) {
	logger.Info().Msg("server router started")
	defer wait.Done()

	defer func() {
		for k, v := range resorceMap {
			v.Close()
			logger.Info().Uint32("uridigest", k).Msg("conn closed")
		}
	}()

	t := time.NewTicker(time.Second * 5)
	for {
		select {
		case <-ctx.Done():
			logger.Info().Msg("context done")
			return
		case <-t.C:
			autoUpdate()
		}
	}
}
func GetResouce(uri uint32) (*grpc.ClientConn, error) {
	if conn, ok := resorceMap[uri]; !ok {
		return nil, ErrResourceNotFound
	} else {
		return conn, nil
	}
}

func dial(addr string) (*grpc.ClientConn, error) {
	conn, err := grpc.Dial(
		addr,
		grpc.WithDefaultServiceConfig(`{"loadBalancingConfig": [{"round_robin":{}}]}`),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		logger.Error().Err(err).Str("addr", addr).Msg("dial error")
		return nil, err
	}
	return conn, nil

}

var updateForFirstTime_fordebug = true

func autoUpdate() {
	// todo
	// load router yaml
	// parse yaml
	// load it to map<uri,addr>
	// range map and check updatetimestap
	// if updatetimestap > now {close conn if exist and connect to resource service}

	// for debug
	if updateForFirstTime_fordebug {
		updateForFirstTime_fordebug = false

		uri := "/test"
		addr := "127.0.0.1:17912"
		conn, err := dial(addr)
		if err != nil {
			logger.Error().Err(err).Str("uri", uri).Str("addr", addr).Msg("resource registraion failed")
		} else {
			uriDigest := crc32.ChecksumIEEE([]byte(uri))
			logger.Info().Str("uri", uri).Uint32("uridigest", uriDigest).Str("addr", addr).Msg("resource registered")
			resorceMap[uriDigest] = conn
		}

	}

}
func init() {
	logger = log.With().Str("module", "server_router").Logger()
	resorceMap = make(map[uint32]*grpc.ClientConn)
}

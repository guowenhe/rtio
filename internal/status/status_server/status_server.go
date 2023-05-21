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
	"errors"
	"flag"
	"fmt"
	"net"
	"os"
	"rtio2/pkg/log"
	ds "rtio2/pkg/rpcproto/devicestatus"
	"strconv"

	"github.com/redis/go-redis/v9"
	"google.golang.org/grpc"
)

var (
	logger log.Logger
	port   = flag.Int("port", 17911, "The server port")
)

const (
	R_DEVICE_K_PREFIX = "ds:"
	R_DEVICE_F_STATUS = "s"
	R_DEVICE_F_ACCESS = "a"

	DEVICEID_LEN_MAX = 30
	DEVICEID_LEN_MIN = 10
	ACCESSID_MIN     = 0
	ACCESSID_MAX     = 20
)

type server struct {
	ds.UnimplementedStatusServiceServer
	rdb *redis.ClusterClient
}

func redisStringAsInt32(v interface{}) (int32, error) {
	s, ok := v.(string)
	if !ok {
		return 0, errors.New("value is not string")
	}

	i, e := strconv.Atoi(s)
	if e != nil {
		return 0, errors.New("value is not a number string")
	}
	return int32(i), nil
}

func (s *server) GetHost(ctx context.Context, req *ds.GetHostReq) (*ds.GetHostResp, error) {
	return &ds.GetHostResp{Host: os.Getenv("HOSTNAME")}, nil
}

func (s *server) GetStatus(ctx context.Context, req *ds.GetStatusReq) (*ds.GetStatusResp, error) {
	logger.Info().Str("DeviceID", req.GetDeviceId()).Msg("GetStatus")

	resp := &ds.GetStatusResp{
		Status:   ds.Status_STATUS_OFFLINE,
		AccessId: int32(0),
		Code:     ds.Code_CODE_FAIL,
	}

	// check deviceid
	if len(req.DeviceId) < DEVICEID_LEN_MIN || len(req.DeviceId) > DEVICEID_LEN_MAX {
		resp.Code = ds.Code_CODE_DEVICEID_INVALAID
		return resp, nil
	}

	val, err := s.rdb.HMGet(ctx, R_DEVICE_K_PREFIX+req.GetDeviceId(),
		R_DEVICE_F_STATUS, R_DEVICE_F_ACCESS).Result()
	if err != nil {
		logger.Error().Err(err).Msg("redis error")
		resp.Code = ds.Code_CODE_FAIL
		return resp, nil
	}

	if len(val) < 2 {
		logger.Error().Msg("redis data incomplete")
		resp.Code = ds.Code_CODE_FAIL
		return resp, nil
	}
	if nil == val[0] && nil == val[1] { // ds not exist in db, means offline
		resp.Status = ds.Status_STATUS_OFFLINE
		resp.Code = ds.Code_CODE_SUCCESS
		return resp, nil
	}
	status, statusErr := redisStringAsInt32(val[0])
	access, accessErr := redisStringAsInt32(val[1])
	if statusErr != nil || accessErr != nil {
		logger.Error().Msg("redis data transfer error")
		resp.Code = ds.Code_CODE_FAIL
		return resp, nil
	}
	resp.Status = ds.Status(status)
	resp.AccessId = access
	resp.Code = ds.Code_CODE_SUCCESS
	return resp, nil
}

func (s *server) SetStatus(ctx context.Context, req *ds.SetStatusReq) (*ds.SetStatusResp, error) {
	logger.Info().Str("DeviceID", req.GetDeviceId()).Msg("SetStatus")

	resp := &ds.SetStatusResp{
		Code: ds.Code_CODE_FAIL,
	}

	// check deviceid
	if len(req.DeviceId) < DEVICEID_LEN_MIN || len(req.DeviceId) > DEVICEID_LEN_MAX {
		logger.Warn().Msg("DEVICEID_INVALAID")
		resp.Code = ds.Code_CODE_DEVICEID_INVALAID
		return resp, nil
	}
	// check status
	if req.Status != ds.Status_STATUS_ONLINE && req.Status != ds.Status_STATUS_OFFLINE {
		logger.Warn().Msg("STATUS_INVALAID")
		resp.Code = ds.Code_CODE_STATUS_INVALAID
		return resp, nil
	}
	// check accessid
	if req.AccessId < ACCESSID_MIN || req.AccessId > ACCESSID_MAX {
		logger.Warn().Msg("ACCESSID_INVALAID")
		resp.Code = ds.Code_CODE_ACCESSID_INVALAID
		return resp, nil
	}
	row, err := s.rdb.HSet(ctx, R_DEVICE_K_PREFIX+req.GetDeviceId(),
		R_DEVICE_F_STATUS, int32(req.Status), R_DEVICE_F_ACCESS, req.AccessId).Result()

	if err != nil {
		logger.Error().Err(err).Msg("redis error")
		resp.Code = ds.Code_CODE_FAIL
		return resp, nil
	}
	logger.Info().Str("DeviceID", req.DeviceId).Int64("row", row).Msg("SetStatus")
	resp.Code = ds.Code_CODE_SUCCESS
	return resp, nil
}

func main() {

	log.OpenFile("status.log")
	defer log.CloseFile()

	logger = log.With().Str("module", "status").Logger()

	logger.Info().Msg("server starting ...")

	// connect to redis
	rdb := redis.NewClusterClient(&redis.ClusterOptions{
		Addrs: []string{
			"redis-0.redis:6379",
			"redis-1.redis:6379",
			"redis-2.redis:6379",
			"redis-3.redis:6379",
			"redis-4.redis:6379",
			"redis-5.redis:6379",
		},
		Username: "beaver",
		Password: "Iapp#9Redis",
	})

	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))

	if err != nil {
		logger.Fatal().Err(err).Msg("failed to listen")
	}
	s := grpc.NewServer()

	ds.RegisterStatusServiceServer(s, &server{rdb: rdb})
	logger.Info().Str("addr", lis.Addr().String()).Msg("server started.")
	if err := s.Serve(lis); err != nil {
		logger.Fatal().Err(err).Msg("failed to serve")
	}
}

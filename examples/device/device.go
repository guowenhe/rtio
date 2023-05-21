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
	"net"
	"os/signal"
	ds "rtio2/internal/access/access_client/devicesession"
	"rtio2/pkg/log"
	"strconv"
	"sync"
	"syscall"
	"time"
)

var logger log.Logger

func handlerAction(req []byte) ([]byte, error) {
	logger.Info().Str("req", string(req)).Msg("")
	return []byte("print started"), nil
}

func handerStatus(ctx context.Context, req []byte) (<-chan []byte, error) {
	logger.Info().Str("req", string(req)).Msg("")
	respChan := make(chan []byte, 1)
	go func(context.Context, <-chan []byte) {

		defer func() {
			close(respChan)
			logger.Info().Msg("Observer exit")
		}()
		t := time.NewTicker(time.Second * 2)
		i := 0
		defer t.Stop()
		for {
			select {
			case <-ctx.Done():
				logger.Info().Msg("ctx.Done()")
				return
			case <-t.C:
				logger.Info().Msg("Notify")
				respChan <- []byte("printing " + strconv.Itoa(i+1) + "%")
				i++
				if i >= 100 {
					return
				}
			}
		}
	}(ctx, respChan)

	return respChan, nil
}

func virtalDeviceRun(ctx context.Context, wait *sync.WaitGroup, deviceID, deviceSecret, serverAddr string) {
	defer wait.Done()

	conn, err := net.DialTimeout("tcp", serverAddr, time.Second*60)
	if err != nil {
		logger.Error().Err(err).Msg("connect server error")
		return
	}
	defer conn.Close()

	deviceSession := ds.NewDeviceSession(conn, deviceID, deviceSecret)
	errChan := make(chan error, 1)
	go deviceSession.Serve(ctx, errChan)

	// URI: /printer/action 0x44d87c69
	deviceSession.RegisterPostHandler(0x44d87c69, handlerAction)
	// URI: /printer/status 0x781495e7
	deviceSession.RegisterObGetHandler(0x781495e7, handerStatus)

	t := time.NewTicker(time.Second * 60)
	defer t.Stop()
EXIT_LOOPY:
	for {
		select {
		case <-ctx.Done():
			logger.Info().Msg("ctx done")
			break EXIT_LOOPY
		case <-t.C:
			logger.Debug().Str("virtalDeviceRun now", time.Now().String())
			resp, err := deviceSession.Post(0x7c88eed8, []byte("this a event"), time.Second*20)
			if err != nil {
				logger.Error().Err(err).Msg("")
			} else {
				logger.Info().Str("resp", string(resp)).Msg("")
			}
		}
	}

	logger.Info().Msg("virtalDeviceRun exit")

}

func main() {

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	logger = log.With().Str("module", "access_client").Logger()
	serverAddr := "localhost:17017"
	wait := &sync.WaitGroup{}
	wait.Add(1)

	go virtalDeviceRun(ctx, wait, "cfa09baa-4913-4ad7-a936-2e26f9671b04", "mb6bgso4EChvyzA05thF9+wH", serverAddr)

	wait.Wait()
	logger.Error().Msg("client exit")

}

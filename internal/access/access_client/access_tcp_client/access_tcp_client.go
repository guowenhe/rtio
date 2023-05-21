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

// func virtalDeviceRun1(ctx context.Context, wait *sync.WaitGroup, deviceID, deviceSecret, serverAddr string) {
// 	defer wait.Done()

// 	conn, err := net.DialTimeout("tcp", serverAddr, time.Second*60)
// 	if err != nil {
// 		logger.Error().Err(err).Msg("connect server error")
// 		return
// 	}
// 	defer conn.Close()

// 	session := ds.NewDeviceSession(conn, deviceID, deviceSecret)
// 	errChan := make(chan error, 1)
// 	go session.Serve(ctx, errChan)

// 	for {
// 		select {
// 		case <-ctx.Done():
// 			logger.Debug().Msg("virtalDeviceRun context done")
// 			return
// 		case err := <-errChan:
// 			logger.Error().Err(err).Msg("session error")
// 		case f := <-session.SSFrames():
// 			logger.Info().Uint16("frameid", f.ID).Msg("virtalDeviceRun")
// 			fmt.Println(string(f.GetRequest()))
// 			f.SetResponse([]byte("this a virtal device resp"))
// 		}
// 	}
// }

func Handler(req []byte) ([]byte, error) {
	logger.Info().Str("req", string(req)).Msg("")
	return []byte("handler-> respone resp"), nil
}

// /test
func Handler_0x7c88eed8(ctx context.Context, req []byte) (<-chan []byte, error) {
	logger.Info().Str("req", string(req)).Msg("")
	respChan := make(chan []byte, 1)
	go func(context.Context, <-chan []byte) {

		defer func() {
			close(respChan)
			logger.Info().Msg("Observer exit")
		}()
		t := time.NewTicker(time.Second * 1)
		i := 0
		defer t.Stop()
		for {
			select {
			case <-ctx.Done():
				logger.Info().Msg("ctx.Done()")
				return
			case <-t.C:
				logger.Info().Msg("Notify")
				respChan <- []byte("word" + strconv.Itoa(i))
				i++
				if i >= 10 {
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

	session := ds.NewDeviceSession(conn, deviceID, deviceSecret)
	errChan := make(chan error, 1)
	go session.Serve(ctx, errChan)

	session.RegisterObGetHandler(0x7c88eed8, Handler_0x7c88eed8)
	session.RegisterGetHandler(0x7c88eed8, Handler)
	session.RegisterPostHandler(0x7c88eed8, Handler)

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
			resp, err := session.Post(0x7c88eed8, []byte("good good bangbangbang"), time.Second*20)
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
	// flag.Usage = func() {
	// 	fmt.Println("./client server:port")
	// }
	// flag.Parse()
	// args := flag.Args()
	// var serverAddr string
	// if len(args) > 1 {
	// 	serverAddr = args[0]
	// } else {
	// 	fmt.Println("./client server:port")
	// 	logger.Error().Msg("server address error")
	// 	return
	// }

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

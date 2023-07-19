//go:build beaglebone
// +build beaglebone

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
	"bytes"
	"context"
	"flag"
	"net"
	"os/signal"
	ds "rtio2/internal/deviceaccess/access_client/devicesession"
	"rtio2/pkg/logsettings"

	"strconv"
	"sync"
	"syscall"
	"time"

	"github.com/rs/zerolog/log"
	"periph.io/x/conn/v3/gpio"
	"periph.io/x/conn/v3/gpio/gpioreg"
	"periph.io/x/host/v3"
)

var printProgress = 0

func handlerAction(req []byte) ([]byte, error) {
	log.Info().Str("req", string(req)).Msg("")
	printProgress = 0
	resp := []byte("unknown cmd")

	if bytes.Equal(req, []byte("start")) {
		resp = []byte("print started")
		go func() {
			host.Init()
			p := gpioreg.ByName("GPIO7")
			t := time.NewTicker(300 * time.Millisecond)
			for l := gpio.High; ; l = !l {
				p.Out(l)
				<-t.C
				printProgress = printProgress + 1
				log.Info().Int("progress", printProgress).Msg("printing")
				if printProgress >= 100 {
					p.Out(gpio.Low)
					break
				}
			}
		}()
	}

	return resp, nil
}

func handerStatus(ctx context.Context, req []byte) (<-chan []byte, error) {
	log.Info().Str("req", string(req)).Msg("")
	respChan := make(chan []byte, 1)
	go func(context.Context, <-chan []byte) {

		defer func() {
			close(respChan)
			log.Info().Msg("Observer exit")
		}()

		respChan <- []byte("printing " + strconv.Itoa(printProgress) + "%") // report first, not wait for timer

		t := time.NewTicker(time.Second * 1)
		defer t.Stop()
		for {
			select {
			case <-ctx.Done():
				log.Info().Msg("ctx.Done()")
				return
			case <-t.C:
				log.Info().Msg("Notify")
				respChan <- []byte("printing " + strconv.Itoa(printProgress) + "%")
				if printProgress >= 100 || printProgress == 0 {
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
		log.Error().Err(err).Msg("connect server error")
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
			log.Info().Msg("ctx done")
			break EXIT_LOOPY
		case <-t.C:
			log.Debug().Str("virtalDeviceRun now", time.Now().String())
			resp, err := deviceSession.Post(0x7c88eed8, []byte("this a event"), time.Second*20)
			if err != nil {
				log.Error().Err(err).Msg("")
			} else {
				log.Info().Str("resp", string(resp)).Msg("")
			}
		}
	}

	log.Info().Msg("virtalDeviceRun exit")

}

func main() {

	logsettings.Set()
	addr := flag.String("addr", "localhost:17017", "the address to connect to")
	flag.Parse()

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	wait := &sync.WaitGroup{}
	wait.Add(1)

	go virtalDeviceRun(ctx, wait, "cfa09baa-4913-4ad7-a936-2e26f9671b04", "mb6bgso4EChvyzA05thF9+wH", *addr)

	wait.Wait()
	log.Error().Msg("client exit")
}

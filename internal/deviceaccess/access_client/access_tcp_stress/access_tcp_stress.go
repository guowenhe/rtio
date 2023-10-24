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
	"bufio"
	"context"
	"errors"
	"flag"
	"os"
	"os/signal"
	ds "rtio2/internal/deviceaccess/access_client/devicesession"
	"rtio2/pkg/logsettings"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/rs/zerolog/log"
)

// system setting: sudo sysctl -w net.ipv4.ip_local_port_range="1025 65534"
const (
	IP_PORT_FIRST = 1025
	IP_PORT_LAST  = 65534
)

func virtalDeviceRun(ctx context.Context, wait *sync.WaitGroup, deviceID, deviceSecret, localAddr, serverAddr string) {
	defer wait.Done()

	log.Info().Str("deviceid", deviceID).Msg("virtalDeviceRun run")

	session, err := ds.ConnectWithLocalAddr(ctx, deviceID, deviceSecret, localAddr, serverAddr)
	if err != nil {
		log.Error().Str("deviceid", deviceID).Err(err).Msg("connection error")
		return
	}
	// GET /test
	session.RegisterGetHandler(0x7c88eed8, func(req []byte) ([]byte, error) {
		log.Info().Str("GET req", string(req)).Msg("")
		return []byte("GET " + deviceID + " resp ok"), nil
	})

	session.Serve(ctx)

	<-ctx.Done()
	log.Info().Msg("ctx done")
	log.Info().Str("deviceid", deviceID).Msg("virtalDeviceRun exit")
}

type Device struct {
	DeviceID     string
	DeviceSecret string
}

func loadDevices(name string) ([]Device, error) {
	f, err := os.Open(name)
	if err != nil {
		return nil, err
	}

	defer f.Close()

	devices := make([]Device, 0, 60000)
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		fields := strings.Split(scanner.Text(), " ")
		if len(fields) < 2 {
			log.Error().Str("reading device fields error, text:", scanner.Text())
			return nil, errors.New("incomplete device data")
		}
		devices = append(devices, Device{fields[0], fields[1]})
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	return devices, nil
}

func main() {

	logsettings.Set()
	serverAddr := flag.String("server", "localhost:17017", "server address")
	localIP := flag.String("local", "", "local ip, default empty. if local ip set, program auto assign port[1025 - 65534] to each device")
	deviceFile := flag.String("file", "", "load all devices from file, max 64510")
	freqLimit := flag.Int("freq", 2000, "connection frequency-limit, default 2000 clients/second, max 20000 clients/second")
	flag.Parse()

	if *freqLimit > 20000 || *freqLimit < 1 {
		log.Error().Msg("connection frequency num error")
		return
	}

	devices, err := loadDevices(*deviceFile)
	if err != nil {
		log.Error().Err(err).Msg("loadDevices error")
		return
	}

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	wait := &sync.WaitGroup{}

	localAddr := ""
	port := IP_PORT_FIRST

	for _, d := range devices {
		if len(*localIP) != 0 {
			localAddr = *localIP + ":" + strconv.Itoa(port)
			port++
		}

		wait.Add(1)
		go virtalDeviceRun(ctx, wait, d.DeviceID, d.DeviceSecret, localAddr, *serverAddr)
		time.Sleep(time.Nanosecond * time.Duration(1000000000 / *freqLimit))
	}

	wait.Wait()
	log.Info().Msg("client exit")

}

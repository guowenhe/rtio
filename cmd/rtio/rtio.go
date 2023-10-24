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
	"os/signal"
	"sync"
	"syscall"

	"rtio2/internal/deviceaccess/access_server/apprpc"
	"rtio2/internal/deviceaccess/access_server/backendconn"
	"rtio2/internal/deviceaccess/access_server/devicetcp"
	"rtio2/internal/useraccess/access_server/usergw"
	"rtio2/pkg/config"
	"rtio2/pkg/logsettings"

	"github.com/google/gops/agent"
	"github.com/rs/zerolog/log"
)

func main() {
	tcpAddr := flag.String("deviceaccess.addr", "0.0.0.0:17017", "address for device conntection")
	userAddr := flag.String("useraccess.addr", "0.0.0.0:17317", "address for user conntection")
	rpcAddr := flag.String("backend.rpc.addr", "0.0.0.0:17217", "(optional) address for app-server conntection")

	logFormat := flag.String("log.format", "text", "text or json, default text")
	logLevel := flag.String("log.level", "warn", " debug, info, warn, error, default warn")
	deviceVerifier := flag.String("backend.deviceverifier", "deviceverifier.rtio:17915", "device verifier service address, for device auth")
	deviceService := flag.String("backend.deviceservice", "deviceservice.rtio:17912", "device service address, device get/post to this service")
	disableDeviceVerify := flag.Bool("disable.deviceverify", false, "no device authentication")
	// disableUserVerify := flag.Bool("disable.userverify", false, "no user token verify")
	disableDeviceService := flag.Bool("disable.deviceservice", false, "disable the backend device services")
	flag.Parse()

	// set configs
	config.StringKV.Set("backend.deviceverifier", *deviceVerifier)
	config.StringKV.Set("backend.deviceservice", *deviceService)
	config.BoolKV.Set("disable.deviceverify", *disableDeviceVerify)
	config.BoolKV.Set("disable.deviceservice", *disableDeviceService)
	// config.BoolKV.Set("disable.userverify", *disableUserVerify)
	config.StringKV.Set("log.format", *logFormat)
	config.StringKV.Set("log.level", *logLevel)

	// set log format and level
	logsettings.Set()

	// show configs
	for _, v := range config.StringKV.List() {
		log.Debug().Msg(v)
	}

	if err := agent.Listen(agent.Options{}); err != nil {
		log.Fatal().Err(err)
	}
	defer agent.Close()

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()
	log.Info().Msg("rtio starting ...")

	backendconn.InitBackendConnn()

	wait := &sync.WaitGroup{}

	sessionMap := &devicetcp.SessionMap{}
	err := devicetcp.InitTCPServer(ctx, *tcpAddr, sessionMap, wait)
	if err != nil {
		log.Error().Err(err).Msg("InitTCPServer error")
		return
	}

	err = apprpc.InitRPCServer(ctx, *rpcAddr, sessionMap, wait)
	if err != nil {
		log.Error().Err(err).Msg("InitRPCServer error")
		return
	}

	err = usergw.InitUserGateway(ctx, *rpcAddr, *userAddr, wait)
	if err != nil {
		log.Error().Err(err).Msg("InitUserGateway error")
		return
	}

	log.Debug().Msg("rtio wait for subroutes")
	wait.Wait()
	log.Info().Msg("rtio stoped")
}

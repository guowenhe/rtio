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
	"os/signal"
	"sync"
	"syscall"

	"rtio2/internal/deviceaccess/access_server/apprpc"
	"rtio2/internal/deviceaccess/access_server/backendconn"
	"rtio2/internal/deviceaccess/access_server/devicetcp"
	"rtio2/internal/useraccess/access_server/usergw"
	"rtio2/pkg/logsettings"

	"github.com/google/gops/agent"
	"github.com/rs/zerolog/log"
)

func main() {

	logsettings.Set()

	flag.Usage = func() {
		fmt.Println("ENVs")
		fmt.Println("  RTIO_LOG_JSON \n        log format: true - json, other - text")
		fmt.Println("  RTIO_LOG_LEVEL \n        log level:  debug, info, warn, error, other - debug")
		fmt.Println("  RTIO_RESOURCE_SERVICE_ADDR \n        resource service address, if empty using  dns-name(service name) [rtio-resource:17912]")
		fmt.Println("  RTIO_AUTH_SERVICE_ADDR \n        auth service address,  if empty using  dns-name(service name) [rtio-auth:17915]")
		fmt.Println("\nFLAGS")
		flag.PrintDefaults()
	}

	tcpAddr := flag.String("tcp", "0.0.0.0:17017", "expose address for device conntection")
	rpcAddr := flag.String("rpc", "0.0.0.0:17217", "expose address for app-server conntection (optional)")
	userAddr := flag.String("user", "0.0.0.0:17317", "expose address for user conntection")
	flag.Parse()

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

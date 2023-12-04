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
	"rtio2/internal/useraccess/access_server/usergw"
	"rtio2/pkg/config"
	"rtio2/pkg/logsettings"

	"sync"
	"syscall"

	"github.com/google/gops/agent"
	"github.com/rs/zerolog/log"
)

func main() {
	rpcAddr := flag.String("backend.rpc.addr", "0.0.0.0:17217", "(optional) address for app-server conntection")
	userAddr := flag.String("useraccess.addr", "0.0.0.0:17317", "address for user conntection")
	logFormat := flag.String("log.format", "text", "text or json, default text")
	logLevel := flag.String("log.level", "warn", " debug, info, warn, error, default warn")
	// disableUserVerify := flag.Bool("disable.userverify", false, "no user token verify")
	flag.Parse()

	// set configs
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
	log.Info().Msg("useraccess starting ...")

	// backendconn.InitBackendConnn()

	wait := &sync.WaitGroup{}

	err := usergw.InitUserGateway(ctx, *rpcAddr, *userAddr, wait)
	if err != nil {
		log.Error().Err(err).Msg("useraccess InitUserGateway error")
		return
	}

	log.Debug().Msg("useraccess wait for subroutes")
	wait.Wait()
	log.Info().Msg("useraccess stoped")
}

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
	"os/signal"
	"rtio2/pkg/log"
	"sync"
	"syscall"

	"rtio2/internal/access/access_server/router"
	"rtio2/internal/access/access_server/server_rpc"
	"rtio2/internal/access/access_server/server_tcp"

	"github.com/google/gops/agent"
)

var logger log.Logger

func main() {

	// log settings
	log.OpenFile("access.log")
	defer log.CloseFile()
	logger = log.With().Str("module", "main").Logger()
	logger.Info().Msg("access starting ...")

	if err := agent.Listen(agent.Options{}); err != nil {
		logger.Fatal().Err(err)
	}
	defer agent.Close()

	// context and signal
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	wait := &sync.WaitGroup{}
	sessionMapChan := make(chan *server_tcp.SessionMap, 1)
	errChan := make(chan error, 2)

	wait.Add(1)
	go router.Serve(ctx, wait)
	wait.Add(1)
	go server_tcp.TCPServe(ctx, "0.0.0.0:17017", sessionMapChan, errChan, wait)
	wait.Add(1)
	go server_rpc.RPCServe(ctx, "0.0.0.0:17217", sessionMapChan, errChan, wait)

	select {
	case <-ctx.Done():
	case err := <-errChan:
		logger.Err(err).Msg("access_server failed")
		if err != nil {
			stop()
		}
	}
	logger.Debug().Msg("access_server wait for subroutes")
	wait.Wait()
	logger.Info().Msg("access_server stoped")
}

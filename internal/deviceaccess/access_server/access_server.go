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
	"rtio2/pkg/logsettings"

	"github.com/rs/zerolog/log"
)

func main() {

	logsettings.Set()
	log.Info().Msg("access starting ...")

	// if err := agent.Listen(agent.Options{}); err != nil {
	// 	log.Fatal().Err(err)
	// }
	// defer agent.Close()

	// // context and signal
	// ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	// defer stop()

	// wait := &sync.WaitGroup{}
	// sessionMapChan := make(chan *server_tcp.SessionMap, 1)
	// errChan := make(chan error, 2)

	// wait.Add(1)
	// go router.Serve(ctx, wait)
	// wait.Add(1)
	// go server_tcp.TCPServe(ctx, "0.0.0.0:17017", sessionMapChan, errChan, wait)
	// wait.Add(1)
	// go server_rpc.InitRPCServer(ctx, "0.0.0.0:17217", sessionMapChan, errChan, wait)

	// select {
	// case <-ctx.Done():
	// case err := <-errChan:
	// 	log.Err(err).Msg("access_server failed")
	// 	if err != nil {
	// 		stop()
	// 	}
	// }
	// log.Debug().Msg("access_server wait for subroutes")
	// wait.Wait()
	// log.Info().Msg("access_server stoped")
}

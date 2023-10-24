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
	"fmt"
	"log"

	da "rtio2/pkg/rpcproto/deviceaccess"
	"rtio2/pkg/rtioutils"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func makeRPCs3(cc *grpc.ClientConn, n int) {
	c := da.NewAccessServiceClient(cc)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	id, err := rtioutils.GenUint32ID()
	fmt.Println("id", id)

	if err != nil {
		fmt.Println(id, err)
		return
	}

	req := &da.Req{
		Uri:      "/test", //0x7c88eed8
		Id:       id,
		DeviceId: "cfa09baa-4913-4ad7-a936-2e26f9671b04",
		Data:     []byte("hello"),
	}

	// resp, err := c.Get(ctx, req)
	resp, err := c.Post(ctx, req)
	if err != nil {
		fmt.Println(id, err)
		return
	}
	fmt.Println("resp:", string(resp.Data))
}

func main() {
	// conn, err := grpc.Dial(
	// 	"dns:///grpc-local:17217",
	// 	grpc.WithDefaultServiceConfig(`{"loadBalancingConfig": [{"round_robin":{}}]}`),
	// 	grpc.WithTransportCredentials(insecure.NewCredentials()),
	// )
	// if err != nil {
	// 	log.Fatalf("did not connect: %v", err)
	// }
	// defer conn.Close()

	conn, err := grpc.Dial("localhost:17217", grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()

	makeRPCs3(conn, 1)
}

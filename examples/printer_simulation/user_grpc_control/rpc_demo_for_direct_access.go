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

	"rtio2/pkg/logsettings"
	da "rtio2/pkg/rpcproto/deviceaccess"
	"rtio2/pkg/rtioutils"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

// func callOneTimes(c da.AccessServiceClient) {
// 	ctx, cancel := context.WithTimeout(context.Background(), time.Second*10)
// 	defer cancel()
// 	r, err := c.Send(ctx, &da.SendReq{Id: 1235})
// 	if err != nil {
// 		log.Fatalf("could not Send: %v", err)
// 	}
// 	log.Printf("resp: %v, %v, %v", r.GetCode(), r.Id, string(r.GetData()))
// }

// func makeRPCs(cc *grpc.ClientConn, n int) {
// 	hwc := da.NewAccessServiceClient(cc)
// 	for i := 0; i < n; i++ {
// 		callOneTimes(hwc)
// 		time.Sleep(time.Millisecond * 500)
// 	}
// }

func makeRPCs2(cc *grpc.ClientConn, n int) {
	c := da.NewAccessServiceClient(cc)

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	id, err := rtioutils.GenUint32ID()
	fmt.Println("id", id)

	if err != nil {
		fmt.Println(id, err)
		return
	}

	req := &da.ObGetReq{
		Uri:      "/printer/status", //0x7c88eed8
		Id:       id,
		DeviceId: "cfa09baa-4913-4ad7-a936-2e26f9671b04",
		Data:     []byte("{\"policy\": \"close-when-100\"}"),
	}

	stream, err := c.ObGet(ctx, req)
	if err != nil {
		fmt.Println(id, err)
		return
	}

	for {
		res, err := stream.Recv()
		if err != nil {
			fmt.Println(id, err, "exit")
			return
		}
		fmt.Println(res.Id, res.Code, res.Fid, string(res.Data))
	}

}

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
		Uri:      "/printer/action",
		Id:       id,
		DeviceId: "cfa09baa-4913-4ad7-a936-2e26f9671b04",
		Data:     []byte("{\"commnd\": \"start\"}"),
	}

	resp, err := c.Post(ctx, req)
	if err != nil {
		fmt.Println(id, err)
		return
	}
	fmt.Println("resp:", string(resp.Data))
}

func main() {

	logsettings.Set()
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

	makeRPCs2(conn, 1)
	// makeRPCs3(conn, 1)
}

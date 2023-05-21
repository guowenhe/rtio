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
	"log"
	"time"

	ds "rtio2/pkg/rpcproto/devicestatus"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

func callOneTimes(c ds.StatusServiceClient) {
	// ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	// defer cancel()
	// r, err := c.GetStatus(ctx, &ds.GetStatusReq{DeviceId: "1234"})
	// if err != nil {
	// 	log.Fatalf("could not GetStatus: %v", err)
	// }
	// log.Printf("GetStatus: %v %v", r.GetStatus(), r.GetAccessId())

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	defer cancel()
	r, err := c.GetHost(ctx, &ds.GetHostReq{})
	if err != nil {
		log.Fatalf("could not GetHost: %v", err)
	}
	log.Printf("GetHost: %s", r.GetHost())
}

func makeRPCs(cc *grpc.ClientConn, n int) {
	hwc := ds.NewStatusServiceClient(cc)
	for i := 0; i < n; i++ {
		callOneTimes(hwc)
		time.Sleep(time.Millisecond * 500)
	}
}

func main() {

	// Make another ClientConn with round_robin policy.
	roundrobinConn, err := grpc.Dial(
		"dns:///grpc-test:17911",
		grpc.WithDefaultServiceConfig(`{"loadBalancingConfig": [{"round_robin":{}}]}`), // This sets the initial balancing policy.
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer roundrobinConn.Close()

	makeRPCs(roundrobinConn, 3000)
}

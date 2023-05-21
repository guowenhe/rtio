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
	"os"
	ds "rtio2/pkg/rpcproto/devicestatus"
	"testing"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/connectivity"
	"google.golang.org/grpc/credentials/insecure"
	"gotest.tools/assert"
)

var client ds.StatusServiceClient

func TestSetStatus(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	defer cancel()
	resp, err := client.SetStatus(ctx, &ds.SetStatusReq{DeviceId: "1234567890", Status: ds.Status_STATUS_ONLINE, AccessId: 2})

	assert.NilError(t, err)
	assert.Equal(t, resp.Code, ds.Code_CODE_SUCCESS, "SetStatus Fail!")
}

func TestGetStatus(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	defer cancel()
	resp, err := client.GetStatus(ctx, &ds.GetStatusReq{DeviceId: "12345678901"})
	assert.NilError(t, err)
	assert.Equal(t, resp.Code, ds.Code_CODE_SUCCESS, "GetStatus Fail!")
	assert.Equal(t, resp.Status, ds.Status_STATUS_OFFLINE)

	resp, err = client.GetStatus(ctx, &ds.GetStatusReq{DeviceId: "1234567890"})
	assert.NilError(t, err)
	assert.Equal(t, resp.Code, ds.Code_CODE_SUCCESS, "GetStatus Fail!")

	t.Logf("GetStatus: %v %v", resp.AccessId, resp.Status)
	assert.Equal(t, resp.Status, ds.Status_STATUS_ONLINE)
	assert.Equal(t, resp.AccessId, int32(2))
}

func TestMain(m *testing.M) {

	addr := flag.String("addr", "localhost:17911", "the address to connect to")
	flag.Parse()
	fmt.Printf("grpc server addr: [%s]\n", *addr)
	conn, err := grpc.Dial(*addr, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		fmt.Printf("grpc.Dial error: %s\n", err)
		return
	}
	defer conn.Close()

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	defer cancel()
	if true == conn.WaitForStateChange(ctx, connectivity.Connecting) {
		fmt.Println("state changed:", conn.GetState())
	}
	if conn.GetState() != connectivity.Ready {
		fmt.Println("server connect fail")
		os.Exit(-1)
	}

	client = ds.NewStatusServiceClient(conn)
	m.Run()
}

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
	"crypto/tls"
	"encoding/json"
	"errors"
	"flag"
	"io"
	"net"
	"net/http"
	"os"
	"rtio2/pkg/logsettings"
	"rtio2/pkg/rtioutils"
	"strconv"
	"strings"
	"time"

	"github.com/rs/zerolog/log"
)

type Device struct {
	DeviceID    string
	DeviceToken string
}

type CoGetResp struct {
	ID   int    `json:"id"`
	Code string `json:"code"`
	Data string `json:"data"`
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
	serverAddr := flag.String("server", "localhost:17317", "server address")
	deviceFile := flag.String("file", "", "load all devices from file, max 64510")
	flag.Parse()

	devices, err := loadDevices(*deviceFile)
	if err != nil {
		log.Error().Err(err).Msg("loadDevices error")
		return
	}

	log.Info().Int("num", len(devices)).Msg("load devices")

	successCount := 0
	timeoutCount := 0

	httpTransport := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}

	for i, d := range devices {
		client := &http.Client{Transport: httpTransport, Timeout: 5 * time.Second}

		id, _ := rtioutils.GenUint32ID()
		url := "http://" + *serverAddr + "/" + d.DeviceID + "/get_handler?uri=%2Ftest&id=" + strconv.FormatUint(uint64(id), 10) + "&data=cmVxIGRhdGE%3d"
		log.Debug().Str("url", url).Msg("req")

		resp, err := client.Get(url)
		if err != nil {
			if err, ok := err.(net.Error); ok && err.Timeout() {
				log.Error().Err(err).Msg("http Get Timeout")
				timeoutCount++
			} else {
				log.Error().Err(err).Msg("http Get error")
			}
			continue
		}
		defer resp.Body.Close()
		body, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Error().Err(err).Msg("http body error")
		}
		log.Debug().Str("body", string(body)).Msg("resp")

		r := &CoGetResp{}
		if err := json.Unmarshal(body, r); err != nil {
			log.Error().Err(err).Msg("json Unmarshal error")
			continue
		}
		if r.Code == "CODE_OK" && r.Data[len(r.Data)-4:] == "IG9r" { // " ok" base64 =>"IG9r"
			successCount++
		}
		if ((i + 1) % 1000) == 0 {
			log.Error().Int("devicenum", i+1).Int("success", successCount).Int("timeout", timeoutCount).Msg("result")

		}
	}

	// resp example:
	// "{\"id\":0, \"code\":\"CODE_OK\", \"data\":\"R0VUIDBmMmViY2IyLWUyMmUtNGNmYy05ZjBkLTc1YzNhZmY5OTgxNiByZXNwIG9r\"}"
	// "{\"id\":0, \"code\":\"CODE_NOT_FOUNT\", \"data\":\"\"}"

	log.Error().Int("devicenum", len(devices)).Int("success", successCount).Int("timeout", timeoutCount).Msg("result")

}

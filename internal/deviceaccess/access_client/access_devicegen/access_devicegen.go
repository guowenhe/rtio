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
	"crypto/rand"
	"encoding/base64"
	"flag"
	"os"
	"rtio2/pkg/logsettings"

	"github.com/gofrs/uuid"
	"github.com/rs/zerolog/log"
)

func createSecret() (string, error) {
	randBytes := make([]byte, 3*9)
	_, err := rand.Read(randBytes)
	if err != nil {
		log.Error().Err(err).Msg("rand.Read error")
		return "", err
	}
	s := base64.StdEncoding.EncodeToString(randBytes)
	return s, nil
}

func main() {
	logsettings.Set()
	deviceFile := flag.String("file", "devices.txt", "save devices to the ")
	num := flag.Int("num", 1, "device num, max 64510")

	flag.Parse()

	if *num > 64510 || *num < 1 {
		log.Error().Msg("device num error, more then 64510 (for voliad ports)")
		return
	}

	f, err := os.OpenFile(*deviceFile, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0666)
	if err != nil {
		log.Error().Err(err).Msg("OpenFile err")
		return
	}
	defer f.Close()

	for i := 0; i < *num; i++ {
		u, err := uuid.NewV4()
		if err != nil {
			log.Error().Err(err).Msg("uuid.NewV4 error")
			break
		}
		secret, err := createSecret()
		if err != nil {
			log.Error().Err(err).Msg("createSecret error")
			break
		}
		f.WriteString(u.String() + " " + secret + "\n")
	}

	log.Info().Int("num", *num).Msg("created")

}

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
package logsettings

import (
	"os"

	"github.com/rs/zerolog"
	"github.com/rs/zerolog/log"
)

func Set() {
	jsonFormat := os.Getenv("RTIO_LOG_JSON") // true, false, other as false
	logLevel := os.Getenv("RTIO_LOG_LEVEL")  // debug, info, warn, error, other as debug

	if jsonFormat != "true" {
		log.Logger = zerolog.New(zerolog.ConsoleWriter{Out: os.Stderr, TimeFormat: "2006-01-02 15:04:05.000"}).With().Caller().Timestamp().Logger()
	} else {
		log.Logger = zerolog.New(os.Stderr).With().Caller().Timestamp().Logger()
	}

	switch logLevel {
	case "debug":
		zerolog.SetGlobalLevel(zerolog.DebugLevel)
	case "info":
		zerolog.SetGlobalLevel(zerolog.InfoLevel)
	case "warn":
		zerolog.SetGlobalLevel(zerolog.WarnLevel)
	case "error":
		zerolog.SetGlobalLevel(zerolog.ErrorLevel)
	default:
		zerolog.SetGlobalLevel(zerolog.DebugLevel)
	}

	zerolog.TimeFieldFormat = zerolog.TimeFormatUnixMs
	// zerolog.DisableSampling(true)
}

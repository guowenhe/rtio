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

// global log for rtio, reference zerolog.log
// Package log provides a global logger for zerolog.
package log

import (
	"context"
	"errors"
	"fmt"
	"os"

	"github.com/rs/zerolog"
)

type Logger = zerolog.Logger

// mainLogger is the global logger.
var mainLogger zerolog.Logger

var logFile *os.File

func init() {
	mainLogger = zerolog.New(zerolog.ConsoleWriter{Out: os.Stderr, TimeFormat: "2006-01-02 15:04:05.000"}).With().Caller().Timestamp().Logger()
	zerolog.SetGlobalLevel(zerolog.DebugLevel)
	zerolog.TimeFieldFormat = zerolog.TimeFormatUnixMs
	zerolog.DisableSampling(true)
}

func OpenFile(name string) error {
	var err error
	if nil == logFile {
		logFile, err = os.OpenFile(name, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0666)
		if err != nil {
			mainLogger.Fatal().Err(err).Msg("Failed to open error log file")
			return err
		}
		// mainLogger = mainLogger.Output(logFile) // save json log to file
	} else {
		err = errors.New("logFile already exist and opened")
		mainLogger.Fatal().Err(err).Msg("")
		return err
	}
	return err
}

func CloseFile() error {
	var err error
	if nil == logFile {
		err = errors.New("logFile no exist")
		mainLogger.Warn().Err(err).Msg("")
	} else {
		if err = logFile.Close(); err != nil {
			mainLogger.Fatal().Err(err).Msg("")
		}
	}
	return err
}

// With creates a child logger with the field added to its context.
func With() zerolog.Context {
	return mainLogger.With()
}

// Hook returns a logger with the h Hook.
func Hook(h zerolog.Hook) zerolog.Logger {
	return mainLogger.Hook(h)
}

// Err starts a new message with error level with err as a field if not nil or
// with info level if err is nil.
//
// You must call Msg on the returned event in order to send the event.
func Err(err error) *zerolog.Event {
	return mainLogger.Err(err)
}

// Debug starts a new message with debug level.
//
// You must call Msg on the returned event in order to send the event.
func Debug() *zerolog.Event {
	return mainLogger.Debug()
}

// Info starts a new message with info level.
//
// You must call Msg on the returned event in order to send the event.
func Info() *zerolog.Event {
	return mainLogger.Info()
}

// Warn starts a new message with warn level.
//
// You must call Msg on the returned event in order to send the event.
func Warn() *zerolog.Event {
	return mainLogger.Warn()
}

// Error starts a new message with error level.
//
// You must call Msg on the returned event in order to send the event.
func Error() *zerolog.Event {
	return mainLogger.Error()
}

// Log starts a new message with no level. Setting zerolog.GlobalLevel to
// zerolog.Disabled will still disable events produced by this method.
//
// You must call Msg on the returned event in order to send the event.
func Log() *zerolog.Event {
	return mainLogger.Log()
}

// Print sends a log event using debug level and no extra field.
// Arguments are handled in the manner of fmt.Print.
func Print(v ...interface{}) {
	mainLogger.Debug().CallerSkipFrame(1).Msg(fmt.Sprint(v...))
}

// Printf sends a log event using debug level and no extra field.
// Arguments are handled in the manner of fmt.Printf.
func Printf(format string, v ...interface{}) {
	mainLogger.Debug().CallerSkipFrame(1).Msgf(format, v...)
}

// Ctx returns the Logger associated with the ctx. If no logger
// is associated, a disabled logger is returned.
func Ctx(ctx context.Context) *zerolog.Logger {
	return zerolog.Ctx(ctx)
}

// func (e *zerolog.Event) Uint32Hex(key string, i uint32) *zerolog.Event {

// 	buf := bytes.NewBuffer([]byte{})
// 	binary.Write(buf, binary.BigEndian, i)
// 	e.Hex(key, buf.Bytes())
// 	return e
// }

package main

import (
	"context"
	"flag"
	"os/signal"
	"rtio2/internal/useraccess/access_server/usergw"
	"rtio2/pkg/logsettings"

	"sync"
	"syscall"

	"github.com/rs/zerolog/log"
)

func main() {

	logsettings.Set()

	rpcAddr := flag.String("rpc", "0.0.0.0:17217", "address to connect device access service")
	userAddr := flag.String("user", "0.0.0.0:17317", "address for user access service")
	flag.Parse()

	log.Info().Msg("useraccess starting ...")

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	wait := &sync.WaitGroup{}
	err := usergw.InitUserGateway(ctx, *rpcAddr, *userAddr, wait)
	if err != nil {
		log.Error().Err(err).Msg("InitUserGateway error")
	}

	log.Debug().Msg("useraccess wait for subroutes")
	wait.Wait()
	log.Info().Msg("useraccess stoped")

}

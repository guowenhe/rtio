package usergw

import (
	"context"
	"net/http"

	"sync"

	da "rtio2/pkg/rpcproto/deviceaccess"

	"github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
	"github.com/rs/zerolog/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/protobuf/reflect/protoreflect"
)

func addHeaders(ctx context.Context, w http.ResponseWriter, resp protoreflect.ProtoMessage) error {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
	w.Header().Set("Access-Control-Allow-Headers", "*")
	return nil
}

func InitUserGateway(ctx context.Context, rpcAddr, gwAddr string, wait *sync.WaitGroup) error {

	conn, err := grpc.DialContext(
		ctx,
		rpcAddr,
		grpc.WithBlock(),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		log.Error().Err(err).Msg("failed to dial server")
		return err
	}
	log.Info().Str("rpcaddr", gwAddr).Msg("connected")

	gwmux := runtime.NewServeMux(
		runtime.WithForwardResponseOption(addHeaders),
	)
	err = da.RegisterAccessServiceHandler(context.Background(), gwmux, conn)
	if err != nil {
		log.Error().Err(err).Msg("failed to register gateway")
		return err
	}

	gwServer := &http.Server{
		Addr:    gwAddr,
		Handler: gwmux,
	}
	log.Info().Str("gwaddr", gwAddr).Msg("gateway started")

	wait.Add(1)
	go func() {
		defer wait.Done()
		err = gwServer.ListenAndServe()
		if err != nil {
			if err == http.ErrServerClosed {
				log.Info().Msg("gateway http closed")
				return
			}
			log.Error().Err(err).Msg("gateway serve failed")
		}
	}()

	go func() {
		<-ctx.Done()
		log.Info().Msg("gateway ctx down")
		gwServer.Shutdown(ctx)
	}()

	return nil
}

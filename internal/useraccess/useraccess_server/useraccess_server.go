package main

import (
	"context"
	"net/http"
	"rtio2/pkg/log"

	da "rtio2/pkg/rpcproto/deviceaccess"

	"github.com/grpc-ecosystem/grpc-gateway/v2/runtime"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/protobuf/reflect/protoreflect"
)

var logger log.Logger

func addHeaders(ctx context.Context, w http.ResponseWriter, resp protoreflect.ProtoMessage) error {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
	w.Header().Set("Access-Control-Allow-Headers", "*")
	return nil
}

func main() {

	// log settings
	log.OpenFile("useraccess.log")
	defer log.CloseFile()
	logger = log.With().Str("module", "main").Logger()
	logger.Info().Msg("useraccess starting ...")

	conn, err := grpc.DialContext(
		context.Background(),
		"0.0.0.0:17217",
		grpc.WithBlock(),
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		logger.Fatal().Err(err).Msg("failed to dial server")
	}

	gwmux := runtime.NewServeMux(
		runtime.WithForwardResponseOption(addHeaders),
	)
	err = da.RegisterAccessServiceHandler(context.Background(), gwmux, conn)
	if err != nil {
		logger.Fatal().Err(err).Msg("failed to register gateway")
	}

	gwServer := &http.Server{
		Addr:    "0.0.0.0:17317",
		Handler: gwmux,
	}
	logger.Info().Msg("gateway started on 17317")

	err = gwServer.ListenAndServe()
	if err != nil {
		logger.Fatal().Err(err).Msg("gateway serve failed")
	}
}

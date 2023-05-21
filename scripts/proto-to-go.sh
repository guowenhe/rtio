#!/bin/bash

WS_ROOT=$(dirname $(dirname `realpath $0`))
echo "proto-to-go's WORK_ROOT:" $WS_ROOT

SOURCES=`find $WS_ROOT/pkg/rpcproto/ -name *.proto`

function _build() {
  for src in ${SOURCES}; do
    echo "protoc ${src}"
    PROTOC_PATH=$(dirname ${src})
    protoc --go_out=$PROTOC_PATH --go_opt=paths=source_relative \
          --go-grpc_out=$PROTOC_PATH  --go-grpc_opt=paths=source_relative --proto_path=$PROTOC_PATH  ${src}

  done
}

function _clean() {
  find $WS_ROOT/pkg/rpcproto/ -name *.pb.go | xargs rm
}


if [ "$1" == "clean" ]; then
  _clean
else
  _build
fi
all: build

build:
	mkdir -p out
	go build -gcflags="all=-N -l" -o ./out/ rtio2/cmd/...

	mkdir -p out/deviceaccess
	go build -gcflags="all=-N -l" -o ./out/deviceaccess rtio2/internal/deviceaccess/...

	mkdir -p out/useraccess
	go build -gcflags="all=-N -l" -o ./out/useraccess rtio2/internal/useraccess/...

	mkdir -p out/examples
	go build -gcflags="all=-N -l" -o ./out/examples/ rtio2/examples/...
	CGO_ENABLED=0 GOOS=linux GOARCH=arm go build -gcflags="all=-N -l" -tags beaglebone -o ./out/examples/printer_beaglebone rtio2/examples/printer_simulation/printer_beaglebone/

clean:
	echo "clean all ..."
	go clean -i rtio2/...
	rm -rf ./out 
	

deps:
	GO111MODULE=on go get -d -v rtio2/...

proto:
	@ if ! which protoc > /dev/null; then \
		echo "error: protoc not installed" >&2; \
		exit 1; \
	fi
	@ if ! which protoc-gen-go > /dev/null; then \
		echo "error: protoc-gen-go not installed" >&2; \
		exit 1; \
	fi
	@ if ! which protoc-gen-go-grpc > /dev/null; then \
		echo "error: protoc-gen-go-grpc not installed" >&2; \
		exit 1; \
	fi
	@ if ! which protoc-gen-grpc-gateway > /dev/null; then \
		echo "error: protoc-gen-grpc-gateway not installed" >&2; \
		exit 1; \
	fi
	go generate rtio2/...

proto-clean:
	./scripts/proto-to-go.sh clean


test:
	go test -cpu 1,4 -timeout 7m rtio2/...


.PHONY: \
	all \
	build \
	clean \
	proto \
	test \
	testrace \
	vet \
	vetdeps

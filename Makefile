all: build

build:
	mkdir -p out
	go build -gcflags="all=-N -l" -o ./out/ rtio2/internal/...

	mkdir -p out/examples
	go build -gcflags="all=-N -l" -o ./out/examples/ rtio2/examples/...

clean:
	go clean -i rtio2/...
	# rm -rf ./out 
	./scripts/proto-to-go.sh clean
	

deps:
	GO111MODULE=on go get -d -v rtio2/...

proto:
	@ if ! which protoc > /dev/null; then \
		echo "error: protoc not installed" >&2; \
		exit 1; \
	fi
	go generate rtio2/...

test:
	go test -cpu 1,4 -timeout 7m rtio2/...

# testsubmodule:
# 	cd security/advancedtls && go test -cpu 1,4 -timeout 7m google.golang.org/grpc/security/advancedtls/...
# 	cd security/authorization && go test -cpu 1,4 -timeout 7m google.golang.org/grpc/security/authorization/...

# testrace:
# 	go test -race -cpu 1,4 -timeout 7m google.golang.org/grpc/...

# testdeps:
# 	GO111MODULE=on go get -d -v -t google.golang.org/grpc/...

# vet: vetdeps
# 	./vet.sh

# vetdeps:
# 	./vet.sh -install

.PHONY: \
	all \
	build \
	clean \
	proto \
	test \
	testrace \
	vet \
	vetdeps

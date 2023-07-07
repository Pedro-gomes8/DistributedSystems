package main

import (
	"SD2023Tp3/server/models"
	server "SD2023Tp3/server/serverRPC"
	"SD2023Tp3/server/toolsTP3/semaphore"
	"SD2023Tp3/server/toolsTP3/terminalParser"
	"flag"
	"fmt"
	"log"
	"net"
	"sync"

	"google.golang.org/grpc"
)

var (
	port = flag.Int("port", 50051, "The server port")
)

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	fmt.Println("Initializing server")
	grpcServer := grpc.NewServer()

	// Core server data structure. Core functionality is in models/models.go
	servBone := &models.Tp3RPCServer{
		Mut:               semaphore.NewWeighted(1),
		RWcurrMutexHolder: &sync.RWMutex{},
		CurrMutexHolder:   -1,
		Served:            make(map[int32]int32),
		ServedMut:         &sync.Mutex{}}

	server.RegisterTp3RPCServer(grpcServer, servBone)

	// Create a thread to handle terminal input
	go terminalParser.TerminalParser(grpcServer, servBone)

	// Start serving
	grpcServer.Serve(lis)
}

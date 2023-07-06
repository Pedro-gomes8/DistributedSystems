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

// Mutex pra liberar o acesso ao processo. Note que eh um semaforo com pesos, pois assim a politica é fifo, permitindo a representacao da fila.
// var mut = semaphore.NewWeighted(1)
// var queueMutex sync.Mutex
var (
	port = flag.Int("port", 50051, "The server port")
)

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	grpcServer := grpc.NewServer()
	servBone := &models.Tp3RPCServer{
		Mut:               semaphore.NewWeighted(1),
		RWcurrMutexHolder: &sync.RWMutex{},
		CurrMutexHolder:   -1,
		Served:            make(map[int32]int32),
		ServedMut:         &sync.Mutex{}}

	server.RegisterTp3RPCServer(grpcServer, servBone)
	fmt.Println("initializing server")
	go terminalParser.TerminalParser(grpcServer, servBone)
	grpcServer.Serve(lis)
}

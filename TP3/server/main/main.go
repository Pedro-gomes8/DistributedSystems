package main

import (
	"SD2023Tp3/server/semaphore"
	server "SD2023Tp3/server/serverRPC"
	"context"
	"flag"
	"fmt"
	"google.golang.org/grpc"
	"log"
	"net"
	"sync"
)

type tp3RPCServer struct {
	server.UnimplementedTp3RPCServer
	mut               *semaphore.Weighted
	currMutexHolder   int32
	rWcurrMutexHolder *sync.RWMutex
}

// Mutex pra liberar o acesso ao processo. Note que eh um semaforo com pesos, pois assim a politica é fifo, permitindo a representacao da fila.
// var mut = semaphore.NewWeighted(1)
// var queueMutex sync.Mutex
var (
	port = flag.Int("port", 50051, "The server port")
)

// Acquires critical section
func (s *tp3RPCServer) grant(processId int32) {
	// Handles the queue manipulation
	// s.queueMutex.Lock()
	// s.queue = append(s.queue, processId)
	// s.queueMutex.Unlock()
	// Handles the 'grant' manipulation
	s.mut.Acquire(context.Background(), 1, processId)
	fmt.Println("semaphore ok")
	s.rWcurrMutexHolder.Lock()
	s.currMutexHolder = processId
	s.rWcurrMutexHolder.Unlock()
}

func (s *tp3RPCServer) Request(ctx context.Context, message *server.ClientMessage) (*server.GrantedMessage, error) {
	fmt.Println("request called")
	processNo := message.GetProcessId()
	s.grant(processNo)
	return &server.GrantedMessage{
		TargetProcessId: processNo,
		Granted:         true}, nil

}

func (s *tp3RPCServer) Release(ctx context.Context, message *server.ClientMessage) (*server.ReleaseResponse, error) {
	processNo := message.GetProcessId()

	s.rWcurrMutexHolder.RLock()
	defer s.rWcurrMutexHolder.RUnlock()

	if s.currMutexHolder == processNo {
		s.mut.Release(1)

		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          true}, nil
	} else {
		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          false}, nil
	}
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	grpcServer := grpc.NewServer()
	server.RegisterTp3RPCServer(grpcServer, &tp3RPCServer{
		mut:               semaphore.NewWeighted(1),
		rWcurrMutexHolder: &sync.RWMutex{},
		currMutexHolder:   -1})
	fmt.Println("initializing server")
	grpcServer.Serve(lis)
}

package main

import (
	server "SD2023Tp3/rpc"
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"sync"

	"golang.org/x/sync/semaphore"
	"google.golang.org/grpc"
)

type tp3RPCServer struct {
	server.UnimplementedTp3RPCServer
	mut             semaphore.Weighted
	queueMutex      sync.Mutex
	queue           []int32
	currMutexHolder int32
}

// Mutex pra liberar o acesso ao processo. Note que eh um semaforo com pesos, pois assim a politica é fifo, permitindo a representacao da fila.
// var mut = semaphore.NewWeighted(1)
// var queueMutex sync.Mutex
var (
	port = flag.Int("port", 50051, "The server port")
)

// Acquires critical section
func (s *tp3RPCServer) grant(processId int32, channel chan bool) {
	defer func() { channel <- true }()

	// Handles the queue manipulation
	s.queueMutex.Lock()
	s.queue = append(s.queue, processId)
	s.queueMutex.Unlock()

	// Handles the 'grant' manipulation
	s.mut.Acquire(context.Background(), 1)
	s.currMutexHolder = processId
}

func (s *tp3RPCServer) Request(ctx context.Context, message *server.ClientMessage) (*server.GrantedMessage, error) {
	processNo := message.GetProcessId()
	ch := make(chan bool)
	// Note that the creation of this routine is not needed.
	go s.grant(processNo, ch)
	<-ch
	return &server.GrantedMessage{
		TargetProcessId: processNo,
		Granted:         true}, nil

}

func (s *tp3RPCServer) Release(ctx context.Context, message *server.ClientMessage) (*server.ReleaseResponse, error) {
	processNo := message.GetProcessId()
	if s.currMutexHolder == processNo {
		s.queueMutex.Lock()
		s.queue = s.queue[1:]
		s.queueMutex.Unlock()
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
	server.RegisterTp3RPCServer(grpcServer, &tp3RPCServer{queue: make([]int32, 0)})
	fmt.Println("initializing server")
	grpcServer.Serve(lis)
}

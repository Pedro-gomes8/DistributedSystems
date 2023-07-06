package models

import (
	server "SD2023Tp3/server/serverRPC"
	"SD2023Tp3/server/toolsTP3/semaphore"
	"context"
	"sync"
)

type Tp3RPCServer struct {
	server.UnimplementedTp3RPCServer
	Mut               *semaphore.Weighted
	CurrMutexHolder   int32
	RWcurrMutexHolder *sync.RWMutex
	Served            map[int32]int32
	ServedMut         *sync.Mutex
}

// Acquires critical section
func (s *Tp3RPCServer) grant(processId int32) {
	// Handles the queue manipulation
	// s.queueMutex.Lock()
	// s.queue = append(s.queue, processId)
	// s.queueMutex.Unlock()
	// Handles the 'grant' manipulation
	s.Mut.Acquire(context.Background(), 1, processId)
	// fmt.Println("semaphore ok")
	s.RWcurrMutexHolder.Lock()
	s.CurrMutexHolder = processId
	s.RWcurrMutexHolder.Unlock()
}

func (s *Tp3RPCServer) ShowQueue() []int32 {
	waiters := s.Mut.ShowQueue()
	return waiters
}

func (s *Tp3RPCServer) ShowServed() map[int32]int32 {
	mpCopy := make(map[int32]int32)
	s.ServedMut.Lock()
	defer s.ServedMut.Unlock()
	for k, v := range s.Served {
		mpCopy[k] = v
	}
	return mpCopy
}

func (s *Tp3RPCServer) Request(ctx context.Context, message *server.ClientMessage) (*server.GrantedMessage, error) {
	// fmt.Println("request called")
	processNo := message.GetProcessId()
	s.grant(processNo)
	return &server.GrantedMessage{
		TargetProcessId: processNo,
		Granted:         true}, nil

}

func (s *Tp3RPCServer) Release(ctx context.Context, message *server.ClientMessage) (*server.ReleaseResponse, error) {
	processNo := message.GetProcessId()

	s.RWcurrMutexHolder.RLock()
	defer s.RWcurrMutexHolder.RUnlock()

	if s.CurrMutexHolder == processNo {

		s.ServedMut.Lock()
		s.Served[processNo]++
		s.ServedMut.Unlock()

		s.Mut.Release(1)

		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          true}, nil
	} else {
		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          false}, nil
	}
}

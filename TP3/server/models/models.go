package models

import (
	server "SD2023Tp3/server/serverRPC"
	"SD2023Tp3/server/toolsTP3/semaphore"
	"context"
	"sync"
)

type Tp3RPCServer struct {
	server.UnimplementedTp3RPCServer
	Mut               *semaphore.Weighted // Main semaphore. It handles the queue manipulation. Check toolsTP3/semaphore/semaphore.go for more details
	CurrMutexHolder   int32               // Id of the process that is currently holding the critical section
	RWcurrMutexHolder *sync.RWMutex       // Read-Write mutex for the 'CurrMutexHolder'. Even though only one thread writes to it, multiple threads can read it
	Served            map[int32]int32     // Map that stores how many times each process has been served
	ServedMut         *sync.Mutex         // Mutex for the 'Served' map. The terminal thread might read it while the Served map is being updated, so a mutex is necessary
}

// Acquires critical section
func (s *Tp3RPCServer) grant(processId int32) {

	// Handles the critical session and queue manipulation. Check toolsTP3/semaphore/semaphore.go for more details. This is the core of the assignment.
	s.Mut.Acquire(context.Background(), 1, processId)

	// After acquiring the critical section, grab the 'current mutex holder' Lock for writing and update the current holder. This is necessary because the 'current mutex holder' can and will be read by other threads

	s.RWcurrMutexHolder.Lock()
	s.CurrMutexHolder = processId
	s.RWcurrMutexHolder.Unlock()
}

// Releases critical section if the client is the current mutex holder. Security is not a concern in this assignment.
func (s *Tp3RPCServer) unlock(processId int32) bool {

	// Locks the Read-Write Mutex for reading. Multiple threads can still read it. But if at least one thread is reading, no thread can write to it
	s.RWcurrMutexHolder.RLock()
	defer s.RWcurrMutexHolder.RUnlock()

	if s.CurrMutexHolder == processId {

		// Update the Served map
		s.ServedMut.Lock()
		s.Served[processId]++
		s.ServedMut.Unlock()

		// Release the critical section
		s.Mut.Release(1, processId)

		return true
	} else {
		return false
	}
}

// Show order of processes waiting for critical section
func (s *Tp3RPCServer) ShowQueue() []int32 {
	waiters := s.Mut.ShowQueue()
	return waiters
}

// Show how many times each process has been served
func (s *Tp3RPCServer) ShowServed() map[int32]int32 {
	mpCopy := make(map[int32]int32)

	// Grabs the Served mutex to guarantee that the map is not being updated while it is being read
	s.ServedMut.Lock()

	defer s.ServedMut.Unlock()
	for k, v := range s.Served {
		mpCopy[k] = v
	}
	return mpCopy
}

// Handle the clients' requests and send a granted message if the critical section was acquired. This is the entry point for the clients when requesting. Note that between calling this function and actually being inserted into the queue, the client might be preempted by another client due to context switching. For that reason, the logging operations are performed when we are manipulating the queue.
func (s *Tp3RPCServer) Request(ctx context.Context, message *server.ClientMessage) (*server.GrantedMessage, error) {
	processNo := message.GetProcessId()
	s.grant(processNo)
	return &server.GrantedMessage{
		TargetProcessId: processNo,
		Granted:         true}, nil

}

// Releases critical section. This is the entry point for the clients when releasing
func (s *Tp3RPCServer) Release(ctx context.Context, message *server.ClientMessage) (*server.ReleaseResponse, error) {
	processNo := message.GetProcessId()

	if OK := s.unlock(processNo); OK {
		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          true}, nil
	} else {
		return &server.ReleaseResponse{
			TargetProcessId: processNo,
			Status:          false}, nil
	}
}

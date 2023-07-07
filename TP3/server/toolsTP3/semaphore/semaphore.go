// This implementation is based on the official semaphore package. It was modified to handle the queue manipulation and to return the order of processes waiting for the critical section.

package semaphore

import (
	"SD2023Tp3/server/toolsTP3/writelog"
	"container/list"
	"context"
	"sync"
)

type waiter struct {
	n     int64
	id    int32
	ready chan<- struct{} // Closed when semaphore acquired.
}

// NewWeighted creates a new weighted semaphore with the given
// maximum combined weight for concurrent access.
func NewWeighted(n int64) *Weighted {
	w := &Weighted{size: n}
	return w
}

// Weighted provides a way to bound concurrent access to a resource.
// The callers can request access with a given weight.
type Weighted struct {
	size    int64
	cur     int64
	mu      sync.Mutex
	waiters list.List // This is the queue
}

// Acquire acquires the semaphore with a weight of n, blocking until resources
// are available or ctx is done. On success, returns nil. On failure, returns
// ctx.Err() and leaves the semaphore unchanged.
//
// If ctx is already done, Acquire may still succeed without blocking.
func (s *Weighted) Acquire(ctx context.Context, n int64, id int32) error {

	// Grab lock to handle queue manipulation.
	s.mu.Lock()

	// Once the lock is grabbed, it is safe to write to the log file.
	writelog.WriteLog("REQUEST", id)

	// If there are enough tokens left and no other process is waiting, grant the request.
	if s.size-s.cur >= n && s.waiters.Len() == 0 {
		s.cur += n
		writelog.WriteLog("GRANTED", id)
		s.mu.Unlock()
		return nil
	}

	if n > s.size {
		// Don't make other Acquire calls block on one that's doomed to fail.
		s.mu.Unlock()
		<-ctx.Done()
		return ctx.Err()
	}
	// Enqueue the request.
	ready := make(chan struct{})
	w := waiter{n: n, id: id, ready: ready}
	elem := s.waiters.PushBack(w)
	// Once enqueued, release queue mutex.
	s.mu.Unlock()

	select {
	case <-ctx.Done():
		err := ctx.Err()
		s.mu.Lock()
		select {
		case <-ready:
			// Acquired the semaphore after we were canceled.  Rather than trying to
			// fix up the queue, just pretend we didn't notice the cancelation.
			err = nil
		default:
			isFront := s.waiters.Front() == elem
			s.waiters.Remove(elem)
			// If we're at the front and there're extra tokens left, notify other waiters.
			if isFront && s.size > s.cur {
				s.notifyWaiters()
			}
		}
		s.mu.Unlock()
		return err

	// If another thread calls Release, it will notify the next process in the queue via this channel. Once notified, it is safe to return the Acquire call and grant the request.
	case <-ready:
		writelog.WriteLog("GRANTED", id)
		return nil
	}
}

// Release releases the semaphore with a weight of n.
func (s *Weighted) Release(n int64, id int32) {
	s.mu.Lock()
	s.cur -= n
	if s.cur < 0 {
		s.mu.Unlock()
		panic("semaphore: released more than held")
	}
	s.notifyWaiters()
	writelog.WriteLog("RELEASED", id)
	s.mu.Unlock()
}

// ShowQueue returns the order of processes waiting for the critical section. It grabs the lock to read the queue.
func (s *Weighted) ShowQueue() []int32 {
	s.mu.Lock()
	defer s.mu.Unlock()
	waiters := make([]int32, 0, s.waiters.Len()) // waiters.Len is O(1) complexity
	for element := s.waiters.Front(); element != nil; element = element.Next() {
		waiters = append(waiters, element.Value.(waiter).id)
	}
	return waiters
}

// Notify via the 'ready' channel the next process in the queue
func (s *Weighted) notifyWaiters() {
	for {
		next := s.waiters.Front()
		if next == nil {
			break // No more waiters blocked.
		}

		w := next.Value.(waiter)
		if s.size-s.cur < w.n {
			// Not enough tokens for the next waiter.  We could keep going (to try to
			// find a waiter with a smaller request), but under load that could cause
			// starvation for large requests; instead, we leave all remaining waiters
			// blocked.
			//
			// Consider a semaphore used as a read-write lock, with N tokens, N
			// readers, and one writer.  Each reader can Acquire(1) to obtain a read
			// lock.  The writer can Acquire(N) to obtain a write lock, excluding all
			// of the readers.  If we allow the readers to jump ahead in the queue,
			// the writer will starve — there is always one token available for every
			// reader.
			break
		}

		s.cur += w.n
		s.waiters.Remove(next)
		close(w.ready)
	}
}

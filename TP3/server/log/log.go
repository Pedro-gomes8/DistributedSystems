package log

import "sync"

type log struct {
	mu sync.Mutex
}

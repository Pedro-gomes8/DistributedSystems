CXX=g++
CXXFLAGS=-Wall -std=c++11 -Wextra -pedantic -pthread

all: producerConsumer

producerConsumer:
	$(CXX) $(CXXFLAGS) producerConsumer.cpp miller-rabin-primality-test.cpp -o producerConsumer

clean:
	rm -f producerConsumer
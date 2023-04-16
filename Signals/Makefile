CXX=g++
CXXFLAGS=-Wall -std=c++11 -Wextra -pedantic

SOURCES=sender.cpp receiver.cpp

all: sender receiver

sender:
	$(CXX) $(CXXFLAGS) sender.cpp -o sender

receiver:
	$(CXX) $(CXXFLAGS) receiver.cpp -o receiver

clean:
	rm -f sender receiver
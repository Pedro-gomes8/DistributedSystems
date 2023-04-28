CXX=g++
CXXFLAGS=-Wall -std=c++11 -Wextra -pedantic -I include/laserpants/dotenv 

all: consumer producer

consumer:
	$(CXX) $(CXXFLAGS) consumer.cpp -o consumer

producer:
	$(CXX) $(CXXFLAGS) producer.cpp -o producer

clean:
	rm -f producer consumer
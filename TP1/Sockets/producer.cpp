#include <sys/socket.h> // for socket(), bind(), connect(), listen(), accept()
#include <netinet/in.h> // for sockaddr_in, inet_ntoa()
#include <arpa/inet.h>  // for htons(), htonl()
#include <unistd.h>     // for close()
#include <iostream>     // for cout, cerr
#include <cstring>      // for memset()
#include "env.h"

unsigned long long generateNumber(unsigned long long previous)
{
    srand(time(0));
    return previous + rand() % 100 + 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: ./producer <quantity of numbers to generate>" << std::endl;
        return -1;
    }

    // Create and setup connection to socket
    int socketProducer = socket(AF_INET, SOCK_STREAM, 0); // IPV4, TCP, default protocol
    if (socketProducer == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress)); // set all bytes to 0
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ADDRESS); // sets the address.
    serverAddress.sin_port = htons(PORT);               // sets the port

    // Connecting to consumer
    while (connect(socketProducer, (struct sockaddr *)&serverAddress, sizeof(serverAddress)))
    {
        perror("Error connecting to server, trying again in 3 seconds");
        sleep(3);
    }

    // Generate number and send them to consumer
    unsigned long long nPrevious = 1;
    int iterations = atoi(argv[1]);

    unsigned long long response;
    for (int i = 0; i < iterations; i++)
    {
        nPrevious = generateNumber(nPrevious);
        std::cout << "Sending " << nPrevious << std::endl;
        send(socketProducer, &nPrevious, sizeof(nPrevious), 0);
        // Waiting for response
        recv(socketProducer, &response, sizeof(response), 0);
        std::cout << "Received response from consumer: " << nPrevious << " is " << (response ? "prime" : "not prime") << std::endl;
    }

    // Terminating
    unsigned long long terminate = 0;
    send(socketProducer, &terminate, sizeof(terminate), 0);
    close(socketProducer);

    return 0;
}

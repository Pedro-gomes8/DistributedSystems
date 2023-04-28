#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include "env.h"

bool isPrime(unsigned long long n)
{
    if (n <= 1)
        return false;
    if (n <= 3)
        return true;

    if (n % 2 == 0 || n % 3 == 0)
        return false;

    for (unsigned long long i = 5; i * i <= n; i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;

    return true;
}

int main()
{

    // Creating socket
    int socketConsumer = socket(AF_INET, SOCK_STREAM, 0);
    if (socketConsumer == -1)
    {
        perror("Error creating socket");
        exit(1);
    }
    // Setting up socket and server
    int optval = 1;
    setsockopt(socketConsumer, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); // allows the port to be reused without waiting for the OS to release it
    struct sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress)); // set all bytes to 0 to avoid initialization problems
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ADDRESS); // sets the address.
    serverAddress.sin_port = htons(PORT);               // sets the port
    bind(socketConsumer, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    listen(socketConsumer, 1); // Allows only 1 connection
    // Preparing to accept connection
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSock = accept(socketConsumer, (struct sockaddr *)&clientAddress, &clientAddressSize); // get new socket, this one is used to communicate with the client

    unsigned long long numberReceived;
    while (1)
    {
        recv(clientSock, &numberReceived, sizeof(numberReceived), 0);
        if (numberReceived == 0)
        {
            std::cout << "Received 0, exiting...\n";
            break;
        }
        bool isPrimeNumber = isPrime(numberReceived);
        std::cout << "Received " << numberReceived << " and it is " << (isPrimeNumber ? "prime" : "not prime") << ". Sending the answer to the producer." << std::endl;
        send(clientSock, &isPrimeNumber, sizeof(isPrimeNumber), 0);
    }
    close(clientSock);
    close(socketConsumer);
    return 0;
}
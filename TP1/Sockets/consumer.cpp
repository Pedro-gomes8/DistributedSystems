#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
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

    struct sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress)); // set all bytes to 0
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // sets the address.
    serverAddress.sin_port = htons(8080);                   // sets the port
    // bind(socketConsumer, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    // Connecting to producer
    if (connect(socketConsumer, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        perror("Error connecting to server");
        exit(1);
    }

    unsigned long long numberReceived;
    while (1)
    {
        recv(socketConsumer, &numberReceived, sizeof(numberReceived), 0);
        if (numberReceived == 0)
        {
            std::cout << "Received 0, exiting...\n";
            break;
        }
        bool isPrimeNumber = isPrime(numberReceived);
        std::cout << "Received " << numberReceived << " and it is " << (isPrimeNumber ? "prime" : "not prime") << ". Sending the answer to the producer." << std::endl;
        send(socketConsumer, &isPrimeNumber, sizeof(isPrimeNumber), 0);
    }

    close(socketConsumer);
    return 0;
}
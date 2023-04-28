#include <unistd.h>
#include <iostream>

unsigned long long generateNumber(unsigned long long previous)
{
    srand(time(0));
    return previous + rand() % 100 + 1;
}

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: ./pipes <quantity of numbers to generate>" << std::endl;
        return -1;
    }
    int fileDescriptors[2];
    // fileDescriptors[1] is for writing and fileDescriptors[0] is for reading.
    if (pipe(fileDescriptors) == -1)
    {
        std::perror("Failed to open pipe");
        return -1;
    };
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        close(fileDescriptors[1]); // Closing the writing end of the pipe, as we are only reading.
        unsigned long long numberReceived;

        while (1)
        {
            read(fileDescriptors[0], &numberReceived, sizeof(long long));
            if (numberReceived == 0)
            {
                close(fileDescriptors[0]);
                std::cout << "Received 0, exiting...\n";
                break;
            }
            bool isPrimeNumber = isPrime(numberReceived);
            std::cout << "Received " << numberReceived << " and it is " << (isPrimeNumber ? "prime" : "not prime") << std::endl;
        }
    }
    else
    {
        // Parent process
        close(fileDescriptors[0]); // Close the reading end of the pipe, as we are only writing.
        unsigned long long nPrevious = 1;
        unsigned long long iterations = std::stoull(argv[1]);
        for (unsigned long long i = 0; i < iterations; i++)
        {
            nPrevious = generateNumber(nPrevious);
            write(fileDescriptors[1], &nPrevious, sizeof(long long));
        }
        unsigned long long terminate = 0;
        write(fileDescriptors[1], &terminate, sizeof(long long));
        close(fileDescriptors[1]);
    }

    return 0;
}
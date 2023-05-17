#include <iostream>
#include <random>
#include <chrono>
#include <queue>
#include <semaphore.h>
#include <mutex>
#include <thread>
#include <chrono>

#include "miller-rabin-primality-test.h"

#define CONSUMETARGET 100000
#define FILENAMEBUFFER "bufferInfo.txt"
#define FILENAMETIMECONFIG "time.csv"
std::mutex bufferManipulationMutex; // unlocked by default
sem_t emptySpaces, filledSpaces;    // need to initialize it

int consumed = 0;

std::vector<int> occupiedBuffer(100000); // keeps track of how many spaces are left after every consume operation

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

class Queue : private std::queue<int>
{

private:
    int currSize;

public:
    using std::queue<int>::queue;

    void push(int value)
    {
        struct timespec timespc;

        clock_gettime(CLOCK_REALTIME, &timespc);
        timespc.tv_sec += 2;
        if (sem_timedwait(&emptySpaces, &timespc) == -1) // timed wait. wait until there's free space on buffer.
        {
        } // time ran out, do nothing and complete execution
        else
        {
            bufferManipulationMutex.lock(); // waits until it is able to manipulate the shared queue.
            std::queue<int>::push(value);
            currSize += 1;
            bufferManipulationMutex.unlock();
            sem_post(&filledSpaces);
        }
    }
    int remove()
    {
        struct timespec timespc;

        clock_gettime(CLOCK_REALTIME, &timespc);
        timespc.tv_sec += 2;
        if (sem_timedwait(&filledSpaces, &timespc) == -1)
        {
            return 0;
        }
        else
        {

            // waits until there's something in the buffer to process
            bufferManipulationMutex.lock(); // waits until it is able to manipulate the shared queue.
            int numberToProcess = std::queue<int>::front();
            std::queue<int>::pop();
            currSize -= 1;
            consumed++;                              // 'consumed' variable is protected
            occupiedBuffer[consumed - 1] = currSize; // Tracking operation, using consumed - 1 as index
            bufferManipulationMutex.unlock();

            sem_post(&emptySpaces);
            return numberToProcess;
        }
    }
};
Queue buffer;

int generateRandom(std::mt19937 &gen, std::uniform_int_distribution<int> &distribution)
{
    int randomNumber = distribution(gen);
    return randomNumber;
}

void producer(std::mt19937 &gen, std::uniform_int_distribution<int> &distribution)
{
    while (consumed < CONSUMETARGET) // only consumer threads can handle the consumed variable, which is protected by a mutex.
    {
        int randomNumber = generateRandom(gen, distribution);
        buffer.push(randomNumber); // The custom Queue class which 'buffer' belongs to handles the synchronization
    }
}

void consumer()
{
    int k_iterations = 20;
    while (consumed < CONSUMETARGET)
    {
        int numberToProcess = buffer.remove(); // buffer.remove handles the 'consume' variable and synchronization between threads. Returns 0 if times out.
        if (numberToProcess)                   // If the timed wait did not time out:
        {
            bool primeNumber = isPrime(numberToProcess, k_iterations);
            std::cout << numberToProcess << " is " << (primeNumber ? "" : "not ") << "prime\n";
        }
    }
}

void assignThreads(std::vector<std::thread> &Producerthreads, std::vector<std::thread> &Consumerthreads, std::mt19937 &gen, std::uniform_int_distribution<int> &distribution)
{
    int produce = Producerthreads.size();
    for (int i = 0; i < produce; i++)
    {
        Producerthreads[i] = std::thread(producer, std::ref(gen), std::ref(distribution));
    }
    int consume = Consumerthreads.size();
    for (int i = 0; i < consume; i++)
    {
        Consumerthreads[i] = std::thread(consumer);
    }
}

void joinThreads(std::vector<std::thread> &Producerthreads, std::vector<std::thread> &Consumerthreads)
{
    for (auto &producerThread : Producerthreads)
        producerThread.join();

    for (auto &consumerThread : Consumerthreads)
        consumerThread.join();
}

void saveToFile(std::vector<int> &occBuffer, int qntProducers, int qntConsumers, int bufferSize)
{
    FILE *fileStream = fopen(FILENAMEBUFFER, "a");
    if (fileStream == NULL)
    {
        std::cout << "Not able to open file\n";
    }
    fprintf(fileStream, "B=%i,C=%i,P=%i\n", bufferSize, qntConsumers, qntProducers);
    for (int i = 0; i < CONSUMETARGET; i++)
        fprintf(fileStream, "%i ", occBuffer[i]);
    fprintf(fileStream, "\n");
    fclose(fileStream);
}
void saveTimeToCsv(std::chrono::duration<double, std::milli> &elapsed, int qntProducers, int qntConsumers, int bufferSize)
{
    FILE *fileStream = fopen(FILENAMETIMECONFIG, "a");

    double duration = elapsed.count();
    fprintf(fileStream, "%i,%i,%i,%f\n", bufferSize, qntConsumers, qntProducers, duration);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: ./producerConsumer <number of producers> <number of consumers> <buffer size>\n";
        exit(1);
    }
    int qntProd = atoi(argv[1]);
    int qntCons = atoi(argv[2]);
    int bufferSize = atoi(argv[3]);

    std::vector<std::thread> producers(qntProd);
    std::vector<std::thread> consumers(qntCons);

    // Setting the seed and using a uniform int distribution alongside the masenne twister 19937 pseudo-random generator as rand() only allows random numbers up to RAND_MAX, which is usually around 32k.
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> distribution(1, 10000000);

    // initializing
    sem_init(&emptySpaces, 0, bufferSize); // initialize semaphore emptySpaces with value bufferSize
    sem_init(&filledSpaces, 0, 0);         // initialize semaphore filledSpaces with value 0;

    auto start = std::chrono::high_resolution_clock::now();
    assignThreads(producers, consumers, gen, distribution);
    joinThreads(producers, consumers);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;

    // Destroy
    // mutex object gets automatically destroyed. different libraries for mutex and semaphores.
    sem_destroy(&emptySpaces);
    sem_destroy(&filledSpaces);

    // Saving interesting results
    saveToFile(occupiedBuffer, qntProd, qntCons, bufferSize);
    saveTimeToCsv(elapsed, qntProd, qntCons, bufferSize);

    return 0;
}
#include <iostream>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

std::atomic_flag lock = ATOMIC_FLAG_INIT;
int resultShared, resultOneThreadOnly = 0;

void acquire(std::atomic_flag &Lock)
{
    // busy wait
    while (Lock.test_and_set(std::memory_order_acquire)) // memory_order_acquire e memory_order_release guarantee that the acquire and release operations are performed in a order with respect to other memory operations. On modern systems, this ensures these operations don't get reordered.
    {
    }
}

void release(std::atomic_flag &Lock)
{
    Lock.clear(std::memory_order_release);
}

char generateNumber()
{
    return (char)((std::rand() % 201) - 100);
}

void populateVector(std::vector<char> &numbers)
{
    for (char &num : numbers)
    {
        num = generateNumber();
    }
}

void calculateEverything(std::vector<char> &vec)
{
    for (auto num : vec)
    {
        resultOneThreadOnly += (int)num;
    }
}

void calculatePartition(std::vector<char> &vec, int start, int end)
{
    int localsum = 0;
    for (int i = start; i < end; i++)
    {
        localsum += (int)vec[i];
    }
    acquire(lock);
    resultShared += localsum;
    release(lock);
}

void assignThreads(std::vector<std::thread> &Kthreads, std::vector<char> &vec, int partitionSize, int length)
{
    int noThreads = Kthreads.size();
    for (int i = 0; i < noThreads; i++)
    {
        int startIndex = i * partitionSize;
        int endIndex = (i == noThreads - 1) ? length : startIndex + partitionSize;
        Kthreads[i] = std::thread(calculatePartition, std::ref(vec), startIndex, endIndex);
    }
}

void joinThreads(std::vector<std::thread> &threads)
{
    for (auto &thread : threads)
    {
        thread.join();
    }
}

int main(int argc, char *argv[])
{
    std::srand(std::time(nullptr));
    if (argc != 3)
    {
        std::cout << "Usage: ./spinlocks <power of numbers to generate> <number of threads>";
        exit(1);
    }
    // Initializing Variables
    int powerNumbers = atoi(argv[1]);
    int noThreads = atoi(argv[2]);
    int qntNumbers = pow(10, powerNumbers);
    int partitionSize = qntNumbers / noThreads;
    std::vector<char> randomNumbers(qntNumbers);
    std::vector<std::thread> threads(noThreads);
    populateVector(randomNumbers);

    std::thread calculateAll(calculateEverything, std::ref(randomNumbers)); // Create a thread to do all the work.

    auto start = std::chrono::high_resolution_clock::now();
    assignThreads(threads, randomNumbers, partitionSize, qntNumbers);
    joinThreads(threads);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end - start;

    calculateAll.join();

    bool resultEquals = (resultShared == resultOneThreadOnly);
    if (!resultEquals)
    {
        std::cout << "ERROR: Something is not right\n";
        exit(1);
    }
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    return 0;
}
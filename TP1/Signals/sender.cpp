/**
 * @file sender.cpp
 * @author Pedro Gomes (pedro.gomes@poli.ufrj.br)
 * @brief Send signal to process
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>
/**
 * @brief Sends signal to a process, if it exists
 *
 *
 * @param argc: Command line PID and signal, respectively
 * @param argv: Usage ./sender PID Signal, or let the program ask for
 * input
 * @return int
 */
int main(int argc, char *argv[])
{
    pid_t pid;
    int signal;
    if (argc == 3)
    {
        pid = atoi(argv[1]);
        signal = atoi(argv[2]);
    }
    else
    {
        std::cout << "Insert the target process id:\n";
        std::cin >> pid;
        std::cout << "Insert the desired signal to be sent:\n";
        std::cin >> signal;
    }
    if (kill(pid, signal) == -1)
    {
        std::perror("the PID argument is not valid");
        exit(1);
    }

    return 0;
}
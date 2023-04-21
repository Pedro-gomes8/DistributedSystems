/**
 * @file receiver.cpp
 * @author Pedro Gomes (pedro.gomes@poli.ufrj.br)
 * @brief Receives signal and does stuff
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>

void handle_usr_signal1()
{
    std::cout << "Hi, you've just activated my first function because you raised " << SIGUSR1 << ", nice!\n";
}

void handle_usr_signal2()
{
    std::cout << "This is my second function. I honestly don't know what to do here but it's the intention that counts\n";
}

void handle_interrupt()
{
    std::cout << "Oooohh, so you want to terminate me, hm? whyyy\n";
    exit(1);
}

void handle_signal(int sig)
{
    switch (sig)
    {
    case SIGUSR1:
        handle_usr_signal1();
        break;

    case SIGUSR2:
        handle_usr_signal2();
        break;

    case SIGINT:
        handle_interrupt();
        break;
    }
}

/**
 * @brief This program is built to receive signals. That's all it does. The signals that are treated inside this file are:
 * SIGUSR1 or, in integer, 10
 * SIGUSR2 or, in integer, 12
 * SIGINT or, in integer, 2
 *
 * @param arc arguments can be used to indicate if the program should operate by busy waiting or block waiting.
 * @param argv 'b' indicates busy waiting. default is block waiting.
 * @return int
 */

int main(int argc, char *argv[])
{
    std::cout << "The signals I'm able to receive are 10, 12 and 2. The last one is an interrupt signal\nBy default, I block myself until I receive a signal. If you want me to do the busy wait thing, please type 'b' as an argument when calling my executable\n";
    std::cout << "Here's my PID to make things easier: " << getpid() << std::endl;
    struct sigaction sa;
    sa.sa_handler = &handle_signal;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    if (argc > 1 && argv[1][0] == 'b')
    {
        while (1)
        {
            std::cout << "Where's my signal?\n";
            sleep(2);
        }
    }
    else
    {
        while (1)
        {

            std::cout << "Waiting signal, please wake me up when it comes\n";
            pause();
        }
    }
    return 0;
}
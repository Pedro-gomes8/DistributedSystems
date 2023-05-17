#include <bits/stdc++.h>
#include "miller-rabin-primality-test.h"
// CODE FOR MILLER ET AL.
// ALL CREDITS GO TO THEM
// time complexity is O(k*logN) where k is the number of iterations that is set in the producerConsumer.cpp file (under the 'consumer' function scope)and N is the number I wish to find the primality of.

// modular exponentiation = x**y % p, but efficient
int power(int x, unsigned int y, int p)
{
    int res = 1;
    x = x % p;
    while (y > 0)
    {
        if (y & 1)
            res = (res * x) % p;
        y = y >> 1; // y = y / 2
        x = (x * x) % p;
    }
    return res;
}

bool millerTest(int d, int n)
{
    int a = 2 + rand() % (n - 4);
    int x = power(a, d, n); // a ** d % n
    if (x == 1 || x == n - 1)
        return true;

    while (d != n - 1)
    {
        x = (x * x) % n;
        d *= 2;
        if (x == 1)
            return false;
        if (x == n - 1)
            return true;
    }
    return false;
}
bool isPrime(int n, int k)
{
    if (n <= 1 || n == 4)
        return false;
    if (n <= 3)
        return true;
    int d = n - 1;
    while (d % 2 == 0)
        d /= 2;

    for (int i = 0; i < k; i++)
    {
        if (!millerTest(d, n))
            return false;
    }
    return true;
}
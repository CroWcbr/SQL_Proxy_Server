#include "../include/Proxy.hpp"

int main()
{
    Proxy proxy(HOST, PORT);

    proxy.Loop();

    return 0;
}
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


#include <string>
#include "cudpclient.h"
#include "ctcpclient.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Please set a host(like \"127.0.0.1\") and protocol(\"u\" or \"t\")\n");
        return -1;
    }

    int port = 10889;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (addr.sin_addr.s_addr == -1)
    {
        printf("Unknown address.\n");
        return -1;
    }

    if (argv[2][0] != 'u' && argv[2][0] != 't')
    {
        printf("Unknown protocol. Should be \"u\" or \"t\".\n");
        return -1;
    }

    CUdpClient udpClient;
    CTcpClient tcpClient;
    CClient *client;

    if (argv[2][0] == 'u')
    {
        udpClient.open(addr);
        client = &udpClient;
    }
    else
    {
        if (tcpClient.open(addr))
        {
            printf("Connection error\n");
            return -1;
        }
        client = &tcpClient;
    }

    for (;;)
    {
        cout << "input string: ";
        string msg;
        getline(cin, msg);

        string answ;

        if (client->sendMessage(msg, answ) < 0)
        {
            printf("Error while receiving of answer. Will exit.\n");
            break;
        }
        cout << "Answer: " << answ << endl;
    }

    return 0;
}


#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>

#include <iostream>


#include "cudpclient.h"
#include "../common/settings.h"

CUdpClient::CUdpClient()
    : m_sock(-1)
{
    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
}

CUdpClient::~CUdpClient()
{
    if (m_sock >= 0) close(m_sock);
}

int CUdpClient::open(const sockaddr_in &server)
{
    // UDP SOCKET
    m_serverAddr = server;
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(m_sock >= 0);
    return m_sock;
}

int CUdpClient::sendMessage(const std::string &msg, std::string &answ)
{
    if (sendUdpMessage(m_sock, m_serverAddr, msg))
        return -1;

    // read
    CUdpMsgStore store;
    struct pollfd polling[1];

    polling[0].revents = 0;
    polling[0].events = POLLIN;
    polling[0].fd = m_sock;

    for (; poll(polling, 1, CUDPCLIENT_TIMEOUT) > 0; )
    {


        char buff[MTU_SIZE];
        // без проверки отправителя
        int buffLen = recv(m_sock, buff, MTU_SIZE, 0);
        if (buffLen < 0) return -1;

        if (buffLen > 0)
        {
            if (store.addPacket(buff, buffLen)) return -1;
        }
        else // buffLen == 0
        {
            int answLen = store.getMessage(answ);
            if (answLen > 0) answ.resize(answLen);

            return answLen;
        }
    }

    return -1; //timeout
}

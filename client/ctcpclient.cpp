#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <assert.h>

#include "ctcpclient.h"
#include "../common/settings.h"

CTcpClient::CTcpClient()
{
}

CTcpClient::~CTcpClient()
{
    if (m_sock >= 0)
    {
        shutdown(m_sock, SHUT_RDWR);
        close(m_sock);
    }
}

int CTcpClient::open(const struct sockaddr_in &addr)
{
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(!(m_sock < 0));
    if (m_sock < 0) return -1;


    if (connect(m_sock, (struct sockaddr *)&addr, sizeof(addr)))
        return -1;

    return 0;
}

int CTcpClient::sendMessage(const std::string &msg, std::string &answ)
{
    if (sendTcpMessage(m_sock, msg))
        return -1;

    // read
    CTcpMsgStore store;
    struct pollfd polling[1];

    polling[0].revents = 0;
    polling[0].events = POLLIN;
    polling[0].fd = m_sock;

    for (; poll(polling, 1, CTCPCLIENT_TIMEOUT) > 0; )
    {

        char buff[MTU_SIZE];

        int buffLen = recv(m_sock, buff, MTU_SIZE, 0);
        if (buffLen <= 0) return -1;


        if (store.addPacket(buff, buffLen)) return -1;

        int answLen = store.getMessage(answ);
        if (answLen > 0) return answLen;
    }

    return -1; //timeout
}

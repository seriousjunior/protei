#ifndef CUDPCLIENT_H
#define CUDPCLIENT_H

#include "cclient.h"
#include "../common/udpmsgtransport.h"

// Таймаут получения ответа (см. poll())
#define CUDPCLIENT_TIMEOUT (3 * 1000)


class CUdpClient : public CClient
{

    struct sockaddr_in m_serverAddr;
    int m_sock;

public:
    CUdpClient();
    ~CUdpClient();
    int open(const struct sockaddr_in &server);
    int sendMessage(const std::string &msg, std::string &answ);
};

#endif // CUDPCLIENT_H

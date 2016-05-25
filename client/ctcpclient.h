#ifndef CTCPCLIENT_H
#define CTCPCLIENT_H

#include <netinet/in.h>
#include "cclient.h"
#include "../common/tcpmsgtransport.h"

// Таймаут получения ответа (см. poll())
#define CTCPCLIENT_TIMEOUT (3 * 1000)

class CTcpClient : public CClient
{
    int m_sock;
public:
    CTcpClient();
    ~CTcpClient();
    int open(const struct sockaddr_in &addr);
    int sendMessage(const std::string &msg, std::string &answ);
};

#endif // CTCPCLIENT_H

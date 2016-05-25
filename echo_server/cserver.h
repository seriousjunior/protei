#ifndef CSERVER_H
#define CSERVER_H

#include <set>
#include <vector>
#include <poll.h>
#include <map>

#include "cservlet.h"
#include "../common/udpmsgtransport.h"
#include "../common/tcpmsgtransport.h"
#include "csockaddrkey.h"

#define CSERVER_TCP_CLIENTS_MAX 20



class CServer
{
    struct pollfd m_pollings[CSERVER_TCP_CLIENTS_MAX + 1 + 1]; // Количество сокетов: max TCP + 1 UDP + 1 TCP
    int m_pollingsNum;
    int m_udpSock;


    std::map<CSockAddrKey, CUdpMsgStore> m_udpMessages;
    std::map<int, CTcpMsgStore> m_tcpMessages;


    int addSockToPoll(int sock);

    int delPolling(int idPoll);

public:
    CServer();

    ~CServer();

    int initialize();

    int exec(CServlet &servlet);
};

#endif // CSERVER_H

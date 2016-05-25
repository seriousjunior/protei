#include <assert.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>

#include "cserver.h"
#include "../common/settings.h"


using namespace std;

CServer::CServer()
    : m_pollingsNum(0)
{
}

CServer::~CServer()
{
    for(int it = 0; it < m_pollingsNum; it ++)
    {
        if (m_pollings[it].fd >= 0)
            close(m_pollings[it].fd);
    }
}

int CServer::addSockToPoll(int sock)
{
    assert(!(sock < 0));
    if (m_pollingsNum >= sizeof(m_pollings) / sizeof(struct pollfd) || sock < 0) return -1;

    memset(m_pollings + m_pollingsNum, 0, sizeof(struct pollfd));
    m_pollings[m_pollingsNum].fd = sock;
    m_pollings[m_pollingsNum].events = POLLIN | POLLRDHUP | POLLHUP;
//    m_pollings[m_pollingsNum].events = POLLIN;

    m_pollingsNum ++;

    return 0;
}

int CServer::delPolling(int idPoll)
{
    if (idPoll < 0 || idPoll >= m_pollingsNum) return -1;

    // Сдвиг массива на 1 элемент назад
    for ( idPoll ++; idPoll < m_pollingsNum; idPoll ++)
    {
        m_pollings[idPoll - 1] = m_pollings[idPoll];
    }

    m_pollingsNum --;

    return 0;
}

int CServer::initialize()
{
    uint16_t port = 10889; // TODO

    // UDP SOCKET
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (addSockToPoll(sock)) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr *)&addr, sizeof(addr)))
    {
        assert(false);
        return -1;
    }

    // TCP SOCKET
    sock = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        assert(false);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr *)&addr, sizeof(addr)))
    {
        assert(false);
        return -1;
    }

    if (listen(sock, CSERVER_TCP_CLIENTS_MAX))
    {
        assert(false);
        return -1;
    }

    if (addSockToPoll(sock)) return -1;

    return 0;
}

int CServer::exec(CServlet &servlet)
{
    assert(!(m_pollingsNum < 1));
    if (m_pollingsNum < 1) return -1;


    for (;;)
    {
        int eventsNum = poll(m_pollings, m_pollingsNum, 10 * 1000);
        if (eventsNum < 0) continue;


        for (int itPoll = 0; itPoll < m_pollingsNum && eventsNum > 0; itPoll ++)
        {
            if (!m_pollings[itPoll].revents) continue;
            eventsNum --;
            const short revents = m_pollings[itPoll].revents;
            m_pollings[itPoll].revents = 0;

            switch(itPoll)
            {
            case 0: //UDP
            {
                if (!(revents & POLLIN)) break; // нет данных для чтения

                struct sockaddr_in sender;
                socklen_t senderLen = sizeof(sender);
                memset(&sender, 0, sizeof(sender));
                char pack[MTU_SIZE];

                int packLen = recvfrom(m_pollings[itPoll].fd, pack, MTU_SIZE, 0,(struct sockaddr *)&sender, &senderLen);

                if (packLen < 0) break;

                map<CSockAddrKey, CUdpMsgStore>::iterator itUdpStore =
                    m_udpMessages.insert(pair<CSockAddrKey, CUdpMsgStore>(CSockAddrKey(sender), CUdpMsgStore())).first;

                CUdpMsgStore &msgStore = (*itUdpStore).second;

                if (!packLen)
                {
                    // Конец сообщения, проверка принятых данных, отправка ответа
                    std::string message;
                    int messageLen = msgStore.getMessage(message);
                    m_udpMessages.erase(itUdpStore);

                    message.resize(messageLen);

                    string answ;
                    if (!servlet(message, answ))
                    {
                        sendUdpMessage(m_pollings[itPoll].fd, sender, answ);
                    }

                }
                else if (msgStore.addPacket(pack, packLen))
                {
                    // неудачное добавление сообщения
                    m_udpMessages.erase(itUdpStore);
                    break;
                }

                break;
            }
            case 1: // TCP main
            {
                struct sockaddr_in sender;
                socklen_t senderLen = sizeof(sender);
                memset(&sender, 0, sizeof(sender));

                int sock = accept(m_pollings[itPoll].fd, (struct sockaddr *)&sender, &senderLen);

                assert(!(sock < 0));
                if (sock < 0) break;

                if (addSockToPoll(sock))
                {
                    shutdown(sock, SHUT_RDWR);
                    close(sock);
                }

                break;
            }
            default: // TCP client
            {
                if ((revents & POLLHUP) || revents & POLLRDHUP)
                {
                    // закрытие соединения
                    m_tcpMessages.erase(m_pollings[itPoll].fd);
                    delPolling(itPoll);
                    shutdown(m_pollings[itPoll].fd, SHUT_RDWR);
                    close(m_pollings[itPoll].fd);
                }
                else if (revents & POLLIN)
                {
                    char pack[MTU_SIZE];
                    int packLen = recv(m_pollings[itPoll].fd, pack, MTU_SIZE, 0);
                    if (packLen < 1) break;

                    map<int, CTcpMsgStore>::iterator itTcpStore =
                        m_tcpMessages.insert(pair<int, CTcpMsgStore>(m_pollings[itPoll].fd, CTcpMsgStore())).first;

                    CTcpMsgStore &msgStore = (*itTcpStore).second;
                    if (msgStore.addPacket(pack, packLen))
                    {
                        // неудачное добавление сообщения
                        m_tcpMessages.erase(itTcpStore);
                        delPolling(itPoll);
                        shutdown(m_pollings[itPoll].fd, SHUT_RDWR);
                        close(m_pollings[itPoll].fd);

                    }
                    else if (msgStore.isMsgReady())
                    {
                        // Конец сообщения, проверка принятых данных, отправка ответа
                        std::string message;
                        msgStore.getMessage(message);
                        msgStore.clear();

                        string answ;
                        if (!servlet(message, answ))
                        {
                            sendTcpMessage(m_pollings[itPoll].fd, answ);
                        }
                    }

                }
                break;
            }
            }

        }
    }

    return 0;
}

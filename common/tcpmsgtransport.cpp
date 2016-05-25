#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

#include "tcpmsgtransport.h"
#include "settings.h"

#define TCPMSGTRANSPORT_HEAD_SIZE 2

using namespace std;

CTcpMsgStore::CTcpMsgStore()
    : m_length(0)
    , m_expected(0)
    , m_headerLength(0)
{
}


int CTcpMsgStore::addPacket(const char *data, int size)
{
    assert(!(!data));
    if (!data || size < 0) return 0;

    // чтение заголовка с длинной сообщения
    if (m_headerLength < TCPMSGTRANSPORT_HEAD_SIZE)
    {
        int it = 0; // количество прочитанных байт в заголовок
        for (; it < size && m_headerLength < TCPMSGTRANSPORT_HEAD_SIZE; it ++, m_headerLength ++)
            m_expected = (m_expected << 8) + data[it]; // big endian

        if (m_headerLength < TCPMSGTRANSPORT_HEAD_SIZE)
        {
            return 0;
        }
        else if (m_expected > MESSAGE_MAX_LEN)
        {
            clear();
            return -1; // много
        }

        size -= it;
        data += it;
    }

    if (m_length + size > m_expected)
    {
        clear();
        return -1; // много
    }

    m_recvPacks.push_back(string(data, size));

    m_length += size;
    return 0;
}

int CTcpMsgStore::getMessage(std::string &message)
{
    if (!isMsgReady()) return -1;

    if (m_recvPacks.size() == 1)
    {
        message = (*m_recvPacks.begin());
    }
    else
    {
        message.resize(m_length);
        int itMsgChar = 0;

        for (list< string >::iterator itPack = m_recvPacks.begin(); itPack != m_recvPacks.end(); itPack ++)
        {
            message.replace(itMsgChar, (*itPack).length(), (*itPack));
            itMsgChar += (*itPack).length();
        }
    }

    return m_expected;
}

bool CTcpMsgStore::isMsgReady() const
{
    return m_length == m_expected;
}

void CTcpMsgStore::clear()
{
    m_length = 0;
    m_expected = 0;
    m_headerLength = 0;
    m_recvPacks.clear();
}



int sendTcpMessage(int sock, const std::string &message)
{
    int msgLength = message.length();
    if (msgLength < 1) return 0;
    if (msgLength > MESSAGE_MAX_LEN) return -1;

    char buff[TCPMSGTRANSPORT_HEAD_SIZE];


    for (int it = TCPMSGTRANSPORT_HEAD_SIZE -1; it >= 0; it --)
    {
        buff[it] = msgLength;
        msgLength >>= 8;
    }


    if (TCPMSGTRANSPORT_HEAD_SIZE != send(sock, buff, TCPMSGTRANSPORT_HEAD_SIZE, 0))
        return -1;

    if (message.length() != send(sock, message.c_str(), message.length(), 0))
        return -1;

    return 0;
}

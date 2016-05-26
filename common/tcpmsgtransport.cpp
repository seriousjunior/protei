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

        //m_headerLength == TCPMSGTRANSPORT_HEAD_SIZE
//        printf("expected %d\n", m_expected);
        try
        {
            m_message.resize(m_expected);
        }
        catch(...)
        {
            clear();
            return -1;
        }

        size -= it;
        data += it;
    }

    if (m_length + size > m_expected)
    {
        clear();
        return -1; // много
    }

    m_message.replace(m_length, size, data);
    //m_message.push_back(string(data, size));

//    printf("length %d, size %d, message %s, data %s\n", m_length, size, m_message.c_str(), data);
    m_length += size;
    return 0;
}

int CTcpMsgStore::getMessage(std::string &message)
{
    if (!isMsgReady()) return -1;

    message = m_message.substr(0, m_length);

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
    m_message.clear();
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
//    printf("%02x %02x\n", buff[0], buff[1]);
//    printf("length %d, message %s\n", message.length(), message.c_str());

    if (TCPMSGTRANSPORT_HEAD_SIZE != send(sock, buff, TCPMSGTRANSPORT_HEAD_SIZE, 0))
        return -1;

    if (message.length() != send(sock, message.c_str(), message.length(), 0))
        return -1;

    return 0;
}

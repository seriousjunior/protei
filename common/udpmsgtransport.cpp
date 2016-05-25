#include <assert.h>
#include <string.h>

#include "udpmsgtransport.h"
#include "crc.h"
#include "settings.h"

using namespace std;


CUdpMsgStore::CUdpMsgStore()
    : m_length(0)
{
}


int CUdpMsgStore::addPacket(const char *data, int size)
{
    assert(!(!data));
    if (!data || size < 0) return 0;
    if (m_length + size > MESSAGE_MAX_LEN + sizeof(uint16_t)) return -1;

    m_recvPacks.push_back(string(data, size));

    m_length += size;
    return 0;
}

int CUdpMsgStore::getMessage(string &message)
{
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

    if (XmodemCrc(message.begin(), message.end())) return -1;

    return (m_length > 0 ? m_length - sizeof(uint16_t) : 0); // - CRC
}


int sendUdpMessage(int sock, const struct sockaddr_in &addr, const string &message)
{
    if (message.size() < 1) return 0; // Нечего отправлять
    if (message.size() > MESSAGE_MAX_LEN) return -1; // Много..

    const uint16_t crc = XmodemCrc(message.begin(), message.end());

    const char *cmessage = message.c_str();
    int itChar = 0;

    for (; itChar + MTU_SIZE < message.size(); itChar += MTU_SIZE)
    {
        if (MTU_SIZE != sendto(sock, cmessage + itChar, MTU_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr)))
            return -1;
    }

    // Отправка хвоста
    char tail[MTU_SIZE];
    int tailLen = message.size() - itChar;

    if (tailLen + sizeof(crc) > MTU_SIZE)
    {
        if (tailLen != sendto(sock, cmessage + itChar, tailLen, 0, (struct sockaddr *)&addr, sizeof(addr)))
            return -1;

        tailLen = 0;
    }
    else
    {
        memcpy(tail, cmessage + itChar, tailLen);
    }

    // Запись CRC (!)MTU_SIZE >= 2
    tail[tailLen ++] = crc >> 8; // CRC HH
    tail[tailLen ++] = crc & 0xFF; // CRC LL
    if (tailLen != sendto(sock, tail, tailLen, 0, (struct sockaddr *)&addr, sizeof(addr)))
        return -1;

    // Отправка признака конца сообщения
    if (sendto(sock, cmessage, 0, 0, (struct sockaddr *)&addr, sizeof(addr)))
        return -1;


    return 0;
}

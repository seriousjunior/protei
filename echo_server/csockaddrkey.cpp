#include <string.h>
#include "csockaddrkey.h"


CSockAddrKey::CSockAddrKey()
{
    memset(&m_addr, 0, sizeof(m_addr));
}

CSockAddrKey::CSockAddrKey(const struct sockaddr_in &addr)
    : m_addr(addr)
{

}

bool CSockAddrKey::operator < (const CSockAddrKey &right) const
{
    if (m_addr.sin_addr.s_addr < right.m_addr.sin_addr.s_addr) return true;
    if (right.m_addr.sin_addr.s_addr < m_addr.sin_addr.s_addr) return true;

    return m_addr.sin_port < right.m_addr.sin_port;
}

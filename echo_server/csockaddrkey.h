#ifndef CSOCKADDRKEY_H
#define CSOCKADDRKEY_H

#include <netinet/in.h>

class CSockAddrKey
{
    struct sockaddr_in m_addr;
public:
    CSockAddrKey();
    CSockAddrKey(const struct sockaddr_in &addr);

    bool operator < (const CSockAddrKey &right) const;
};

#endif // CSOCKADDRKEY_H

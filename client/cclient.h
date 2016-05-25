#ifndef CCLIENT_H
#define CCLIENT_H

#include <string>

class CClient
{
public:
    virtual ~CClient();
    virtual int sendMessage(const std::string &msg, std::string &answ) = 0;
};

#endif // CCLIENT_H

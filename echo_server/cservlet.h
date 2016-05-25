#ifndef CSERVLET_H
#define CSERVLET_H

#include <string>

class CServlet
{
public:
    virtual int operator()(const std::string &msg, std::string &answ) = 0;
    virtual ~CServlet();
};

#endif // CSERVLET_H

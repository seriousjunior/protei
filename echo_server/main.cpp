#include <iostream>
#include <assert.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>

#include <regex>
#include <string>
#include "cserver.h"
#include "cmsganalyzer.h"


using namespace std;

int main()
{
    class CAnalyzeServlet : public CServlet
    {
        CMsgAnalyzer m_analyzer;
    public:
        ~CAnalyzeServlet()
        {

        }

        int operator()(const std::string &msg, std::string &answ)
        {
            //printf("test\n");
            m_analyzer.analyzeMsg(msg);
            answ = msg;
            return 0;
        }
    };

    CAnalyzeServlet servlet;


    CServer server;
    server.initialize();
    server.exec(servlet);

    return 0;
}


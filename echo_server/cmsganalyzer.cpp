#include <set>
#include <assert.h>
#include <regex>
#include <stdio.h>

#include "cmsganalyzer.h"

using namespace std;
CMsgAnalyzer::CMsgAnalyzer()
{

}

int CMsgAnalyzer::analyzeMsg(const std::string &msg)
{
    //printf(">MESSAGE %s\n", msg.c_str());

    printf("======= New message =======\n");
    multiset<double> digits;
    double sum = 0;

    smatch res;
    regex pattern("[\\+-]?[\\d]+(\\.\\d+)?|[\\+-]?\\.\\d+");


    string::const_iterator begin = msg.begin();
    string::const_iterator end = msg.end();

    for(; regex_search(begin, end, res, pattern); begin += res.position() + res[0].length())
    {
        double digit;
        try
        {
            digit = std::stod(res[0], 0);
        }
        catch(...)
        {
            assert(false);
            continue; // ой, всё
        }

        if (digit >= 0 && digit <= 9)
        {
            sum += digit;
            digits.insert(digit);
        }

    }

    if (digits.size() > 0)
    {
        printf("sum: %f, min: %f, max: %f\n", sum, *digits.begin(), *digits.rbegin());
        printf("founded digits:");
        for(multiset<double>::reverse_iterator itDigit = digits.rbegin(); itDigit != digits.rend(); itDigit ++)
        {
            printf(" %f", *itDigit);
        }
        printf("\n");
    }
    else
    {
        printf("acceptable digits weren\'t founded\n");
    }

    return 0;
}

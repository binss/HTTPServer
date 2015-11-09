/***********************************************************
 * @FileName:      Request.h
 * @Author:        binss
 * @Create:        2015-09-13 20:58:56
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/

#ifndef  __REQUEST_H__
#define  __REQUEST_H__

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <vector>
#include <unordered_map>
#include "Logger.h"

using namespace std;
using namespace boost;


class Request
{
public:
    Request();
    unordered_map<string, string> &GetHeader(){ return header_; };
    int Parse(char *buf, int len);
    int Reset();

    unordered_map<string, string> header_;
    string data_;
    Logger logger_;
};

#endif

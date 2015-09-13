/***********************************************************
 * @FileName:      RequestHandler.h
 * @Author:        binss
 * @Create:        2015-09-13 20:58:56
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/

#ifndef  __REQUEST_HANDLER_HPP__
#define  __REQUEST_HANDLER_HPP__

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace boost;


class RequestHandler
{
public:
    RequestHandler();
    unordered_map<string, string> &GetHttpHeader(){ return header_; };
    int ParseRequest(char *buf, int len);

    unordered_map<string, string> header_;
    string data_;
};

#endif

/***********************************************************
* FileName:      ResponseHandler.h
* Author:        binss
* Create:        2015-10-29 09:19:53
* Description:   No Description
***********************************************************/


#ifndef  __RESPONSE_HANDLER_HPP__
#define  __RESPONSE_HANDLER_HPP__

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;


class ResponseHandler
{
public:
    ResponseHandler();
    int InitResponse(unordered_map<string, string> &request_header);
    int GetTime(char * time, int length);
    int BuildResponse();
    string & GetResponse();
    unordered_map<string, string> header_;
    string data_;
    string response_;
};

#endif

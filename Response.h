/***********************************************************
* FileName:      Response.h
* Author:        binss
* Create:        2015-10-29 09:19:53
* Description:   No Description
***********************************************************/


#ifndef  __RESPONSE_H__
#define  __RESPONSE_H__

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;


class Response
{
public:
    Response();
    int Init(unordered_map<string, string> &request_header);
    int GetTime(char * time, int length);
    int Build();
    string & GetStr();
    int Reset();


    unordered_map<string, string> header_;
    string data_;
    string response_str_;

};

#endif

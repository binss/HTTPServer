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
#include "CacheManager.h"

using namespace std;

class Response
{
public:
    Response();
    int Init(unordered_map<string, string> &request_header);
    int GetTime(char * time, int length);
    int Build();
    int Reset();
    int LoadData(string uri);
    char * GetBuffer();
    int GetBufferLength();

private:
    unordered_map<string, string> header_;
    int type_;
    char * buffer_;
    int buffer_length_;
    int buffer_size_;
    string protocol_;
    Cache *cache_;
    map<string, int> DATA_TYPES;

};

#endif

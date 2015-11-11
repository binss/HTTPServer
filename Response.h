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
#include "Logger.h"

using namespace std;

class Response
{
public:
    Response();
    ~Response();
    int Init(unordered_map<string, string> &request_header);
    int Build();
    int Reset();
    int LoadData(string uri);
    char * GetBuffer();
    int GetBufferLength();
    int SetCookie(const char *name, const char *value, string expires, const char *domain=NULL, const char *path=NULL, bool secure=false);
    int GetType();
    int UriDecode(string uri);
    void SetFile(string path);
    void SetCode(int code);
    string GetTarget();
private:
    unordered_map<string, string> header_;
    int type_;
    char * buffer_;
    int buffer_length_;
    int buffer_size_;
    string uri;
    string path_;
    string target_;
    Cache *cache_;
    Logger logger_;
    int code_;
    string reason_;
};

#endif

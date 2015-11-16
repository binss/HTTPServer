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
    int Init(Request &request);
    int BuildHeader();
    int Build();
    int Reset();

    int SetCookie(const char *name, const char *value, string expires, const char *domain=NULL, const char *path=NULL, bool secure=false);
    void SetFile(string path);
    void SetCode(int code);

    char * GetBuffer() { return buffer_; }
    int GetBufferLength() { return buffer_length_; }
    int GetType() { return type_; }
    bool GetKeepAlive() { return keep_alive_; }
    string GetTarget() { return target_; }
    int GetCode() { return code_; }
    int GetContentLength() { return size_; }

private:
    int UriDecode(string uri);


private:
    unordered_map<string, string> header_;
    string method_;
    string protocol_;
    string etag_;

    int code_;
    int type_;
    uLong size_;
    unsigned char * data_;
    string target_;
    bool compress_;
    bool keep_alive_;

    char * buffer_;
    int buffer_length_;
    int buffer_size_;

    Cache *cache_;
    Logger logger_;
};

#endif

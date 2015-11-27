/***********************************************************
* FileName:      Response.h
* Author:        binss
* Create:        2015-10-29 09:19:53
* Description:   No Description
***********************************************************/


#ifndef  __RESPONSE_H__
#define  __RESPONSE_H__

#include "CacheManager.h"
#include "Logger.h"

typedef unordered_map<string, string> SSMap;

using namespace std;

class Response
{
public:
    Response();
    ~Response();
    int Init(SSMap *header, SSMap *cookie, Meta *meta);
    int Build();
    int Reset();

    Byte * GetBuffer() { return buffer_; }
    int GetBufferLength() { return buffer_length_; }
    string GetTarget() { return target_; }
    int GetContentLength() { return size_; }

private:
    int BuildHeader();
    int DecodeTarget();
    int LoadCache(string & path, int type);
    int AllocBuffer();
// user interface
public:
    int SetCookie(const char *name, const char *value, string expires, const char *domain=NULL, const char *path=NULL, bool secure=false);
    void SetFile(string path);
    void SetCode(int code);
    void SetRawString(string str);

public:
    int CODE;
    int TYPE;

private:
    SSMap header_;
    SSMap * request_header_;
    SSMap * request_cookie_;
    Meta * meta_;

    uLong size_;
    Byte * data_;
    string target_;

    Byte * buffer_;
    int buffer_length_;
    int buffer_size_;

    Byte *raw_;
    Logger logger_;
};

#endif

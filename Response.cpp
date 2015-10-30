/***********************************************************
* FileName:      Response.cpp
* Author:        binss
* Create:        2015-10-29 09:19:33
* Description:   No Description
***********************************************************/

#include <time.h>
#include "Response.h"

Response::Response()
{
}


int Response::Init(unordered_map<string, string> &request_header)
{
    if(request_header["request_header"] == "keep-alive")
    {
        header_["Connection"] = "keep-alive";
    }
    header_["Set-Cookie"] = "dudu";
    header_["Server"] = "Dudu Server/0.1";
    char time_buf[128];
    int ret = GetTime(time_buf, 128);
    if( 0 == ret )
    {
        header_["Date"] = string(time_buf);
        header_["Expires"] = string(time_buf);

    }

    response_str_ = "HTTP/1.0 200 OK\r\n";
    return 0;
}

int Response::GetTime(char * time_buf, int length)
{
    if(time_buf == NULL)
    {
        return -1;
    }
    time_t now_time;
    time(&now_time);
    struct tm *now_time_ptr = localtime(&now_time);
    strftime(time_buf, length, "%a %b %d %T %Y GMT", now_time_ptr);
    return 0;
}

int Response::Build()
{
    for(unordered_map<string, string>::iterator iter = header_.begin(); iter != header_.end(); iter++)
    {
        response_str_ += (*iter).first + ": " + (*iter).second + "\r\n";
    }
    printf("[response header]\n%s\n", response_str_.c_str());
    response_str_ += "\r\n";
    response_str_ += "OK\r\n";
    return 0;
}

// temp interface
string & Response::GetStr()
{
    return response_str_;
}


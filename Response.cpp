/***********************************************************
* FileName:      Response.cpp
* Author:        binss
* Create:        2015-10-29 09:19:33
* Description:   No Description
***********************************************************/

#include <time.h>
#include <stdlib.h>
#include "Response.h"

Response::Response()
{
}


int Response::Init(unordered_map<string, string> &request_header)
{
    if(request_header["Connection"] == "keep-alive")
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

    printf("[debug]%s %s %s\n", request_header["method"].c_str(), request_header["url"].c_str(), request_header["protocol"].c_str());

    if(request_header["protocol"] == "HTTP/1.1")
    {
        header_["Transfer-Encoding"] = "chunked";
    }


    response_str_ = "HTTP/1.1 200 OK\r\n";
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
    // printf("[response header]\n%s\n", response_str_.c_str());
    response_str_ += "\r\n";

    string content = "<a href=\"http://www.baidu.com\">hello</a>";
    char content_length[20];
    sprintf(content_length, "%lx\r\n", content.length());

    response_str_ += string(content_length);
    response_str_ += content + "\r\n";
    response_str_ += "0\r\n\r\n";
    return 0;
}

// temp interface
string & Response::GetStr()
{
    return response_str_;
}

int Response::Reset()
{
    header_.clear();
    data_.clear();
    response_str_.clear();
    return 0;
}

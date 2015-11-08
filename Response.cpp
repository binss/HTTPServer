/***********************************************************
* FileName:      Response.cpp
* Author:        binss
* Create:        2015-10-29 09:19:33
* Description:   No Description
***********************************************************/

#include <time.h>
#include <stdlib.h>
#include <cstdio>
#include <map>
#include <string>
#include <cstring>
#include <sstream>

#include "GlobalUtil.h"
#include "Mapper.h"
#include "Response.h"

#define NORMAL_BUFFER_SIZE 1024 * 1024 + 2048
#define BIG_BUFFER_SIZE 1024 * 1024 * 10 + 2048

template<class TO, class TI>
inline TO ToType(const TI& input_obj)
{
    stringstream ss;
    ss << input_obj;

    TO output_obj;
    ss >> output_obj;

    return output_obj;
}

Response::Response()
{
    cache_ = NULL;
    buffer_ = NULL;
    buffer_length_ = 0;
    buffer_size_ = 0;
    type_ = 0;
    logger_ = new Logger("Response", DEBUG, false);
}

Response::~Response()
{
    delete []buffer_;
    buffer_ = NULL;
    header_.clear();
}

int Response::Init(unordered_map<string, string> &request_header)
{
    Reset();

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


    LoadData(request_header["uri"]);

    if(request_header["protocol"] == "HTTP/1.1" && type_ < 10)
    {
        header_["Transfer-Encoding"] = "chunked";
    }
    if(type_ >= 10)
    {
        header_["Content-Length"] = ToType<string, int>(cache_->size_);
        header_["Accept-Ranges"] = "bytes";
    }

    return 0;
}

int Response::LoadData(string uri)
{
    string path;
    string file_type;
    if(uri != "")
    {
        if(uri.find("..") == string::npos)
        {

            if(uri.length() > 4)
            {
                file_type = uri.substr(uri.length() - 3);
                type_ = Mapper::GetInstance()->GetContentType(file_type);
            }
            else
            {
                type_ = 1;
            }
            printf("[debug]file type: %s[%d]\n", file_type.c_str(), type_);

            path = Mapper::GetInstance()->GetURI(uri, type_);
            cache_ = CacheManager::GetInstance()->GetCache(path, type_);
            if( NULL == cache_)
            {
                type_ = 1;
                path = Mapper::GetInstance()->GetURI("/404/", type_);
            }

            switch(type_)
            {
                case 1:
                {
                    header_["Content-Type"] = "text/html";
                    break;
                }
                case 10:
                {
                    header_["Content-Type"] = "text/javascript";
                    break;
                }
                case 11:
                {
                    header_["Content-Type"] = "text/css";
                    break;
                }
                case 20:
                case 21:
                case 22:
                case 23:
                {
                    header_["Content-Type"] = "image/" + file_type;
                    break;
                }
                default:
                {
                    printf("[error]can not recognize type: %s, set to default type\n", file_type.c_str());
                    header_["Content-Type"] = "text/html";
                }
            }

        }
        else
        {
            // 禁止.. 防止目录外文件被返回 直接返回403
            type_ = 1;
            path = Mapper::GetInstance()->GetURI("/403/", type_);

        }
    }
    else
    {
        type_ = 1;
        path = Mapper::GetInstance()->GetURI("/404/", type_);
    }
    printf("[debug]uri:%s, path:%s\n", uri.c_str(), path.c_str());
    cache_ = CacheManager::GetInstance()->GetCache(path, type_);
    if(NULL == cache_)
    {
        printf("[error]GetCache error\n");
        return -1;
    }

    if(type_ < 20)
    {
        if(buffer_ == NULL)
        {
            buffer_ = new char[NORMAL_BUFFER_SIZE];
            buffer_size_ = NORMAL_BUFFER_SIZE;
        }
    }
    if(type_ >= 20)
    {
        if(buffer_ == NULL || buffer_size_ < BIG_BUFFER_SIZE)
        {
            delete []buffer_;
            buffer_ = new char[BIG_BUFFER_SIZE];
            buffer_size_ = BIG_BUFFER_SIZE;
        }
    }

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
    string header_str = "HTTP/1.1 200 OK\r\n";
    for(unordered_map<string, string>::iterator iter = header_.begin(); iter != header_.end(); ++ iter)
    {
        header_str += (*iter).first + ": " + (*iter).second + "\r\n";
    }
    header_str += "\r\n";

    memcpy(buffer_, header_str.c_str(), header_str.length());
    buffer_length_ += header_str.length();
    if(type_ >= 10)
    {
        memcpy(buffer_ + buffer_length_, cache_->data_, cache_->size_);
        buffer_length_ += cache_->size_;
    }
    else
    {
        char content_length[20];
        sprintf(content_length, "%x\r\n", cache_->size_);
        memcpy(buffer_ + buffer_length_, content_length, strlen(content_length));
        buffer_length_ += strlen(content_length);

        memcpy(buffer_ + buffer_length_, cache_->data_, cache_->size_);
        buffer_length_ += cache_->size_;


        char buffer_end[20] = "\r\n0\r\n\r\n";
        memcpy(buffer_ + buffer_length_, buffer_end, strlen(buffer_end));
        buffer_length_ += strlen(buffer_end);
    }
    return 0;
}

char * Response::GetBuffer()
{
    return buffer_;
}

int Response::GetBufferLength()
{
    // printf("[debug]%d\n", buffer_length_);
    return buffer_length_;
}


int Response::Reset()
{
    header_.clear();
    buffer_length_ = 0;
    memset(buffer_, 0, buffer_size_);
    return 0;
}


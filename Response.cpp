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

template<class TO, class TI>
inline TO ToType(const TI& input_obj)
{
    stringstream ss;
    ss << input_obj;
    TO output_obj;
    ss >> output_obj;
    return output_obj;
}

Response::Response():logger_("Response", DEBUG, true)
{
    cache_ = NULL;
    buffer_ = NULL;
    buffer_length_ = 0;
    buffer_size_ = 0;
    type_ = 0;
    code_ = 200;
}

Response::~Response()
{
    delete [] buffer_;
    buffer_ = NULL;
    header_.clear();
}

int Response::Init(unordered_map<string, string> &request_header)
{
    if(request_header["Connection"] == "keep-alive")
    {
        header_["Connection"] = "keep-alive";
    }
    header_["Server"] = "Dudu Server/0.1";

    header_["Date"] = GetTime(0);
    header_["Expires"] = GetTime(0);
    logger_<<DEBUG<<"URI: "<<request_header["uri"]<<endl;

    UriDecode(request_header["uri"]);

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

int Response::SetCookie(const char *name, const char *value, string expires, const char *domain, const char *path, bool secure)
{
    stringstream buffer;
    buffer<<name<<"="<<value<<"; "<<"expires="<<expires<<"; ";
    if( NULL != domain )
    {
        buffer<<"domain="<<domain<<"; ";
    }
    if( NULL != path)
    {
        buffer<<"path="<<path<<"; ";
    }
    if( secure )
    {
        buffer<<"secure; ";
    }
    string key = "Set-Cookie";
    while( "" != header_[key])
    {
        key += " ";
    }
    header_[key] = buffer.str();
    return 0;
}

void Response::SetFile(string path)
{
    path_ = "/" + path;
    cache_ = CacheManager::GetInstance()->GetCache(path_, type_);
    if(NULL == cache_)
    {
        logger_<<ERROR<<"Get cache instance error!"<<endl;
    }
}

void Response::SetCode(int code)
{
    code_ = code;
}

int Response::UriDecode(string uri)
{
    string file_type;
    if(uri != "")
    {
        if(uri.find("..") == string::npos)
        {

            if(uri.length() > 4)
            {
                file_type = uri.substr(uri.length() - 3);
                type_ = Mapper::GetInstance()->GetContentType(file_type);
                cache_ = CacheManager::GetInstance()->GetCache(uri, type_);
                if( NULL == cache_)
                {
                    type_ = 0;
                    target_ = "/404/";
                }
                else
                {
                    target_ = uri;
                }
            }
            else
            {
                type_ = 0;
                target_ = uri;
            }


        }
        else
        {
            // 禁止.. 防止目录外文件被返回 直接返回403
            target_ = "/403/";
        }
    }
    else
    {
        target_ = "/404/";
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

    logger_<<DEBUG<<"URI: "<<uri<<" target: "<<target_<<" File type: "<<file_type<<"["<<type_<<"]"<<endl;

    return 0;
}

int Response::Build()
{
    switch(type_)
    {
        case 1:
        {
            header_["Content-Type"] = "text/html"; break;
        }
        case 10:
        {
            header_["Content-Type"] = "text/javascript"; break;
        }
        case 11:
        {
            header_["Content-Type"] = "text/css"; break;
        }
        case 20:
        {
            header_["Content-Type"] = "image/png"; break;
        }
        case 21:
        {
            header_["Content-Type"] = "image/jpg"; break;
        }
        case 22:
        {
            header_["Content-Type"] = "image/gif"; break;
        }
        case 23:
        {
            header_["Content-Type"] = "image/ico"; break;
        }
        default:
        {
            // logger_<<ERROR<<"Can not recognize type: "<<file_type<<" set to default type(text)"<<endl;
            header_["Content-Type"] = "text/html";
        }
    }



    // cache_ = CacheManager::GetInstance()->GetCache(path_, type_);
    // if(NULL == cache_)
    // {
    //     logger_<<ERROR<<"Get cache instance error!"<<endl;
    //     return -1;
    // }

    reason_ = Mapper::GetInstance()->GetReason(code_);
    string method = "HTTP/1.1";
    string header_str = method + " " + reason_ + "\r\n";
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
    return buffer_length_;
}

int Response::GetType()
{
    return type_;
}

string Response::GetTarget()
{
    return target_;
}

int Response::Reset()
{
    type_ = 0;
    code_ = 200;
    header_.clear();
    buffer_length_ = 0;
    memset(buffer_, 0, buffer_size_);
    return 0;
}


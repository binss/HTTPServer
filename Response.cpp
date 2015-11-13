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
#include <zlib.h>

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
    compress_ = false;
    cache_ = NULL;
    buffer_ = NULL;
    buffer_length_ = 0;
    buffer_size_ = 0;
    type_ = 0;
    code_ = 0;
}

Response::~Response()
{
    delete [] buffer_;
    buffer_ = NULL;
    header_.clear();
}

int Response::Init(unordered_map<string, string> &request_header)
{
    UriDecode(request_header["uri"]);


    method_ = request_header["protocol"];

    if(request_header["Accept-Encoding"].find("gzip") != string::npos)
    {
        compress_ = true;
    }

    if(request_header["Connection"] == "keep-alive")
    {
        keep_alive_ = true;
    }

    // 接收范围请求
    // header_["Accept-Ranges"] = "bytes";

    return 0;
}

int Response::UriDecode(string uri)
{
    string file_type = "default";
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

    // 分配内存
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

int Response::BuildHeader()
{
    header_["Server"] = "Dudu Server/0.1";
    header_["Date"] = GetTime(0);
    // 失效时间
    header_["Expires"] = GetTime(0);

    switch(type_)
    {
        case 0:
        case 1:
        {
            header_["Content-Type"] = "text/html; charset=UTF-8"; break;
        }
        case 10:
        {
            header_["Content-Type"] = "text/javascript; charset=UTF-8"; break;
        }
        case 11:
        {
            header_["Content-Type"] = "text/css; charset=UTF-8"; break;
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
            logger_<<ERROR<<"Can not recognize type["<<type_<<"], set to default type[0]"<<endl;
            header_["Content-Type"] = "text/html";
        }
    }

    if(method_ == "HTTP/1.0")
    {
        if(keep_alive_)
        {
            header_["Connection"] = "keep-alive";
        }
        if(compress_)
        {
            header_["Content-Length"] = ToType<string, uLong>(cache_->compress_size_);
            header_["Content-Encoding"] = "gzip";
        }
        else
        {
            header_["Content-Length"] = ToType<string, uLong>(cache_->size_);
        }
    }
    else if(method_ == "HTTP/1.1")
    {
        // 对于页面，分chunked发送
        // TODO：分chucked
        if(type_ == 0)
        {
            header_["Transfer-Encoding"] = "chunked";
            if(compress_)
            {
                header_["Content-Encoding"] = "gzip";
            }
        }
        else
        {
            if(compress_)
            {
                header_["Content-Length"] = ToType<string, uLong>(cache_->compress_size_);
                header_["Content-Encoding"] = "gzip";
            }
            else
            {
                header_["Content-Length"] = ToType<string, uLong>(cache_->size_);
            }
        }
        // Cache-Control: no-cache
        // 对于Connection: close的请求，需要在回复后close
    }
    return 0;
}


int Response::Build()
{
    if( 0 == code_ )
    {
        logger_<<ERROR<<"The response code should be set!"<<endl;
        return -1;
    }

    if(compress_ && COMPRESS_ON && cache_->compress_size_ > 0 && cache_->compress_data_ != NULL)
    {
        compress_ = true;
    }
    else
    {
        compress_ = false;
    }

    BuildHeader();


    string reason = Mapper::GetInstance()->GetReason(code_);
    string method = "HTTP/1.1";
    string header_str = method + " " + reason + "\r\n";
    for(unordered_map<string, string>::iterator iter = header_.begin(); iter != header_.end(); ++ iter)
    {
        header_str += (*iter).first + ": " + (*iter).second + "\r\n";
    }
    header_str += "\r\n";

    memcpy(buffer_, header_str.c_str(), header_str.length());
    buffer_length_ += header_str.length();

    uLong size;
    unsigned char *data;
    if(compress_)
    {
        size = cache_->compress_size_;
        data = cache_->compress_data_;
    }
    else
    {
        size = cache_->size_;
        data = cache_->data_;
    }

    if(type_ == 0)
    {
        char content_length[20];

        sprintf(content_length, "%lx\r\n", size);
        memcpy(buffer_ + buffer_length_, content_length, strlen(content_length));
        buffer_length_ += strlen(content_length);

        memcpy(buffer_ + buffer_length_, data, size);
        buffer_length_ += size;


        char buffer_end[20] = "\r\n0\r\n\r\n";
        memcpy(buffer_ + buffer_length_, buffer_end, strlen(buffer_end));
        buffer_length_ += strlen(buffer_end);
    }
    else
    {
        memcpy(buffer_ + buffer_length_, data, size);
        buffer_length_ += size;
    }
    return 0;
}

int Response::Reset()
{
    compress_ = false;
    type_ = 0;
    code_ = 0;
    target_ = "";
    header_.clear();
    buffer_length_ = 0;
    memset(buffer_, 0, buffer_size_);
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
    path = "/" + path;
    cache_ = CacheManager::GetInstance()->GetCache(path, type_);
    if(NULL == cache_)
    {
        logger_<<ERROR<<"Get cache instance error!"<<endl;
    }
}

void Response::SetCode(int code)
{
    code_ = code;
}

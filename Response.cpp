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
    keep_alive_ = false;
    data_ = NULL;
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
    delete [] raw_;
    raw_ = NULL;
    header_.clear();
}

int Response::Init(Request &request)
{
    UriDecode(request.URI);

    unordered_map<string, string> & header = request.HEADER;
    method_ = request.METHOD;
    protocol_ = request.PROTOCOL;

    if(header["Accept-Encoding"].find("gzip") != string::npos)
    {
        compress_ = true;
    }


    if(header["Connection"] == "close")
    {
        keep_alive_ = false;
    }
    else if(header["Connection"] == "keep-alive" || protocol_ == "HTTP/1.1")
    {
        keep_alive_ = true;
    }

    etag_ = header["If-None-Match"];

    // 接收范围请求
    // header_["Accept-Ranges"] = "bytes";
    if(type_ > 0)
    {
        SetFile(target_);
    }
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
                Cache *cache = CacheManager::GetInstance()->GetCache(uri, type_);
                if(NULL == cache)
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
    // header_["Expires"] = GetTime(EXPIRES_TIME);

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
            header_["Content-Type"] = "text/html; charset=UTF-8";
        }
    }

    if(protocol_ == "HTTP/1.0")
    {
        if(keep_alive_)
        {
            header_["Connection"] = "keep-alive";
        }

        if(compress_)
        {
            header_["Content-Encoding"] = "gzip";
        }

        if(type_ == 0)
        {
            // 立即过期，不缓存
            header_["Expires"] = "-1";
        }
        else
        {
            header_["Expires"] = GetTime(MAX_AGE);
        }

        header_["Content-Length"] = ToType<string, uLong>(size_);
    }
    else if(protocol_ == "HTTP/1.1")
    {
        if(!keep_alive_)
        {
            header_["Connection"] = "close";
        }

        // 对于页面，分chunked发送 TODO：分chucked
        if(type_ == 0)
        {
            header_["Transfer-Encoding"] = "chunked";
            if(compress_)
            {
                header_["Content-Encoding"] = "gzip";
            }
            // 不缓存页面
            header_["Cache-Control"] = "no-cache";
        }
        else
        {
            if(compress_)
            {
                header_["Content-Encoding"] = "gzip";
            }
            // 设置cache时间
            if(MAX_AGE > 0)
            {
                header_["Cache-Control"] = "max-age=" + ToType<string, int>(MAX_AGE);
            }
            else
            {
                header_["Cache-Control"] = "no-cache";
            }

            // 判断etag
            if(etag_ == header_["ETag"])
            {
                header_["Content-Length"] = "0";
                code_ = 304;
            }

            header_["Content-Length"] = ToType<string, uLong>(size_);

        }
    }

    return 0;
}

int Response::Build()
{
    if( 0 == code_ )
    {
        logger_<<WARNING<<"The response code should be set!"<<endl;
        code_ = 500;
        keep_alive_ = false;
    }

    if(NULL == data_)
    {
        logger_<<WARNING<<"The response data should be set!"<<endl;
        code_ = 500;
        keep_alive_ = false;
    }
    else
    {
        BuildHeader();

    }


    // 填充header
    string reason = Mapper::GetInstance()->GetReason(code_);
    string protocol = "HTTP/1.1";
    string header_str = protocol + " " + reason + "\r\n";
    for(unordered_map<string, string>::iterator iter = header_.begin(); iter != header_.end(); ++ iter)
    {
        header_str += (*iter).first + ": " + (*iter).second + "\r\n";
    }
    header_str += "\r\n";

    memcpy(buffer_, header_str.c_str(), header_str.length());
    buffer_length_ += header_str.length();

    if(code_ == 304 || code_ == 500)
    {
        return 0;
    }


    // 填充data
    if(type_ == 0)
    {
        char content_length[20];

        sprintf(content_length, "%lx\r\n", size_);
        memcpy(buffer_ + buffer_length_, content_length, strlen(content_length));
        buffer_length_ += strlen(content_length);

        memcpy(buffer_ + buffer_length_, data_, size_);
        buffer_length_ += size_;


        char buffer_end[20] = "\r\n0\r\n\r\n";
        memcpy(buffer_ + buffer_length_, buffer_end, strlen(buffer_end));
        buffer_length_ += strlen(buffer_end);
    }
    else
    {
        memcpy(buffer_ + buffer_length_, data_, size_);
        buffer_length_ += size_;
    }
    cout<<buffer_<<endl;
    return 0;
}

int Response::Reset()
{
    compress_ = false;
    keep_alive_ = false;
    type_ = 0;
    code_ = 0;
    target_ = "";
    header_.clear();
    buffer_length_ = 0;
    memset(buffer_, 0, buffer_size_);
    return 0;
}

// ------------------------------------------
// ----------- User interfaces ---------------
// ------------------------------------------

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
    Cache * cache = CacheManager::GetInstance()->GetCache(path, type_);
    if(NULL == cache)
    {
        logger_<<ERROR<<"Get cache instance error!"<<endl;
    }
    else
    {
        if(COMPRESS_ON && compress_ && cache->compress_size_ > 0 && cache->compress_data_ != NULL)
        {
            compress_ = true;
            size_ = cache->compress_size_;
            data_ = cache->compress_data_;
        }
        else
        {
            compress_ = false;
            size_ = cache->size_;
            data_ = cache->data_;
        }
        header_["ETag"] = cache->etag_;
    }
}

void Response::SetRawString(string str)
{
    compress_ = false;
    size_ = str.size();
    raw_ = new unsigned char[size_];
    memcpy(raw_, str.c_str() , size_);
    data_ = raw_;
}


void Response::SetCode(int code)
{
    code_ = code;
}

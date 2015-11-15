/***********************************************************
 * @FileName:      Request.cpp
 * @Author:        binss
 * @Create:        2015-09-13 20:59:10
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/


#include <stdlib.h>
#include <sstream>

#include "Request.h"
#include "GlobalUtil.h"

template<class TO, class TI>
inline TO ToType(const TI& input_obj)
{
    stringstream ss;
    ss << input_obj;
    TO output_obj;
    ss >> output_obj;
    return output_obj;
}

Request::Request():logger_("Request", DEBUG, true)
{
    buffer_ = new char[REQUEST_BUFFER_SIZE];
}

Request::~Request()
{
    delete [] buffer_;
    buffer_ = NULL;
}

int Request::Parse(int length)
{
    if( NULL == buffer_ || length <= 0 )
    {
        logger_<<ERROR<<"Request error!"<<endl;
        return -1;
    }
    string str(buffer_);

    // 先切出header和data
    vector<string> parts = split(str, "\r\n\r\n");

    if(parts.size() != 2)
    {
        logger_<<ERROR<<"Part decode error! lenght:"<<parts.size()<<endl;
        return -2;
    }
    // 解析header
    vector<string> lines = split(parts[0], "\r\n");
    if(lines.size() <= 0)
    {
        logger_<<ERROR<<"Header decode error! header:"<<parts[0]<<endl;
        return -3;
    }
    // 对于第一行作特殊处理
    {
        vector<string> metas = split(lines[0], " ");
        if(metas.size() != 3)
        {
            logger_<<ERROR<<"Header meta decode error! meta:"<<lines[0]<<endl;
            return -2;
        }
        METHOD = metas[0];
        PROTOCOL = metas[2];

        // decode uri
        vector<string> uri_tokens = split(metas[1], "?");
        switch(uri_tokens.size())
        {
            case 2:
            {
                vector<string> paras = split(uri_tokens[1], "&");
                for(unsigned int i=0; i<paras.size(); i++)
                {
                    vector<string> token = split(paras[i], "=");
                    if(token.size() == 2)
                    {
                        GET[token[0]] = token[1];
                    }
                    else
                    {
                        logger_<<ERROR<<"URI token decode error! token:"<<paras[i]<<endl;
                    }
                }
            }
            case 1:
            {
                URI = uri_tokens[0];
                break;
            }
            default:
            {
                logger_<<ERROR<<"URI decode error! uri:"<<metas[1]<<endl;
            }

        }
    }

    smatch token;
    regex reg("(.+): *(.+)");
    for (size_t i = 1; i < lines.size(); ++ i)
    {
        regex_match(lines[i], token, reg);
        if(token.size() == 3)
        {
            if(token[1] == "Cookie")
            {
                DecodeCookie(token[2]);
            }
            else
            {
                HEADER[token[1]] = token[2];
            }
        }
        else
        {
            logger_<<ERROR<<"Header line decode error! meta:"<<token[0]<<endl;
        }
    }

    if(METHOD == "POST" && HEADER.find("Content-Length") != HEADER.end())
    {
        if(parts[1].length() != ToType<unsigned int, string>(HEADER["Content-Length"]))
        {
            logger_<<WARNING<<"The length of data["<<parts[1].length()<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
        }
        data_ = parts[1];
    }

    // TODO
    // 处理If-Modified-Since -返回304

    return 0;
}

int Request::DecodeCookie(string cookie_str)
{
    vector<string> cookies = split(cookie_str, ";");
    regex reg(" *(.+)= *(.+)");
    smatch token;
    for(unsigned int i=0; i<cookies.size(); i++)
    {
        regex_match(cookies[i], token, reg);

        if(token.size() == 3)
        {
            COOKIE[token[1]] = token[2];
        }
        else
        {
            logger_<<ERROR<<"cookie token decode error! token:"<<cookies[i]<<endl;
        }
    }
    return 0;
}

int Request::Reset()
{
    data_.clear();
    HEADER.clear();
    GET.clear();
    POST.clear();
    COOKIE.clear();
    memset(buffer_, 0, REQUEST_BUFFER_SIZE);
    return 0;
}

char * Request::GetBuffer()
{
    return buffer_;
}

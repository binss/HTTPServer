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

    int data_length = length;
    char delim[] = "\r\n\r\n";
    data_ = SplitBuffer(buffer_, data_length, delim, strlen(delim));
    if(data_ == NULL)
    {
        logger_<<ERROR<<"Part decode error! buffer:"<<buffer_<<endl;
        return -2;
    }
    string header(buffer_);
    // 解析header
    vector<string> lines = Split(header, "\r\n");
    if(lines.size() <= 0)
    {
        logger_<<ERROR<<"Header decode error! header:"<<header<<endl;
        return -3;
    }
    // 对于第一行作特殊处理
    {
        vector<string> metas = Split(lines[0], " ");
        if(metas.size() != 3)
        {
            logger_<<ERROR<<"Header meta decode error! meta:"<<lines[0]<<endl;
            return -2;
        }
        METHOD = metas[0];
        RAW_URI = metas[1];
        PROTOCOL = metas[2];

        // decode uri
        vector<string> uri_tokens = Split(RAW_URI, "?");
        switch(uri_tokens.size())
        {
            case 2:
            {
                vector<string> paras = Split(uri_tokens[1], "&");
                for(unsigned int i=0; i<paras.size(); i++)
                {
                    vector<string> token = Split(paras[i], "=");
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

    if(METHOD == "POST")
    {
        if(HEADER["Content-Length"] != "")
        {
            if(data_length != ToType<int, string>(HEADER["Content-Length"]))
            {
                logger_<<WARNING<<"The length of data["<<data_length<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
            }
        }

        if(HEADER["Content-Type"] != "")
        {
            DecodeData(data_);
        }
    }
    return 0;
}

int Request::DecodeCookie(string cookie_str)
{
    vector<string> cookies = Split(cookie_str, ";");
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

int Request::DecodeData(string data)
{
    // application/x-www-form-urlencoded
    // multipart/form-data; boundary=----WebKitFormBoundaryoUZxy8WfgYJUqTaA
    if(HEADER["Content-Type"] == "application/x-www-form-urlencoded")
    {
        vector<string> paras = Split(data, "&");
        for(unsigned int i=0; i<paras.size(); i++)
        {
            vector<string> token = Split(paras[i], "=");
            if(token.size() == 2)
            {
                POST[token[0]] = token[1];
            }
            else
            {
                logger_<<ERROR<<"Data token decode error! token:"<<paras[i]<<endl;
            }
        }
    }
    // else if...
    else
    {
        regex reg("(.+); *boundary=(.+)");
        smatch token;
        if(regex_match(HEADER["Content-Type"], token, reg))
        {
            if(token[1] == "multipart/form-data")
            {
                // Chrome && Safari
                string data_reg_str = "-*" + token[2].str() + "\r\nContent-Disposition: (.*); name=\"(.*)\"\r\n\r\n(.*)";
                regex data_reg(data_reg_str);
                for(sregex_iterator iter(data.cbegin(), data.cend(), data_reg), end; iter != end; ++iter)
                {
                    if(iter->format("$1") == "form-data")
                    {
                        POST[iter->format("$2")] = iter->format("$3");
                    }
                    // logger_<<DEBUG<<"type["<<iter->format("$1")<<"] ["<<iter->format("$2")<<"]=["<<iter->format("$3")<<"]"<<endl;
                }
            }
        }
    }
    // logger_<<ERROR<<"POST "<<POST<<endl;
    // logger_<<WARNING<<HEADER["Content-Type"]<<endl;

    return 0;
}


int Request::Reset()
{
    // data_.clear();
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

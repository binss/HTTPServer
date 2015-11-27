/***********************************************************
 * @FileName:      Request.cpp
 * @Author:        binss
 * @Create:        2015-09-13 20:59:10
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/

#include <regex>
#include "Request.h"
#include "GlobalUtil.h"

Request::Request():logger_("Request", DEBUG, true)
{
    header_length_ = 0;
    buffer_size_ = REQUEST_DEFAULT_BUFFER_SIZE;
    buffer_length_ = 0;
    buffer_ = new Byte[buffer_size_];
    memset(&DATA, 0, sizeof(DATA));
}

Request::~Request()
{
    delete [] buffer_;
    buffer_ = NULL;
}

int Request::Parse(int length)
{
    buffer_length_ = length;
    if( NULL == buffer_ || length <= 0 )
    {
        logger_<<ERROR<<"Request error!"<<endl;
        return E_Decode_Error;
    }
    DATA.length = length;
    char delim[] = "\r\n\r\n";
    DATA.pointer = SplitBuffer(buffer_, DATA.length, delim, strlen(delim));
    header_length_ = DATA.pointer - buffer_;
    if(DATA.pointer == NULL)
    {
        logger_<<ERROR<<"Part decode error!"<<endl;
        return E_Decode_Error;
    }
    string header = ToString(buffer_);
    // 解析header
    vector<string> lines = Split(header, "\r\n");
    if(lines.size() <= 0)
    {
        logger_<<ERROR<<"Header decode error! header:"<<header<<endl;
        return E_Decode_Error;
    }
    // 对于第一行作特殊处理
    {
        vector<string> metas = Split(lines[0], " ");
        if(metas.size() != 3)
        {
            logger_<<ERROR<<"Header meta decode error! meta:"<<lines[0]<<endl;
            return E_Decode_Error;
        }
        META.METHOD = metas[0];
        META.RAW_URI = metas[1];
        META.PROTOCOL = metas[2];

        // decode uri
        vector<string> uri_tokens = Split(META.RAW_URI, "?");
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
                META.URI = uri_tokens[0];
                break;
            }
            default:
            {
                logger_<<ERROR<<"URI decode error! uri:"<<metas[1]<<endl;
                return E_Decode_Error;
            }

        }
    }

    // decode header
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
    DecodeHeader();

    if(META.METHOD == "POST")
    {
        if(HEADER["Content-Length"] != "")
        {
            if(DATA.length != ToType<int, string>(HEADER["Content-Length"]))
            {
                logger_<<WARNING<<"The length of data["<<DATA.length<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
                return E_Request_Not_Complete;
            }
            if(HEADER["Content-Type"] != "")
            {
                int ret = DecodeData();
                if(ret != E_Suc)
                {
                    return ret;
                }
            }
            else
            {
                logger_<<ERROR<<"Content-Type is null!"<<endl;
                return E_Decode_Error;
            }
        }

    }
    return E_Suc;
}

int Request::Append(int new_length)
{
    DATA.pointer = buffer_ + header_length_;
    DATA.length = new_length - header_length_;
    if(DATA.length != ToType<int, string>(HEADER["Content-Length"]))
    {
        logger_<<WARNING<<"The length of data["<<DATA.length<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
        return E_Request_Not_Complete;
    }
    if(HEADER["Content-Type"] != "")
    {
        int ret = DecodeData();
        if(ret != E_Suc)
        {
            return ret;
        }
    }
    else
    {
        logger_<<ERROR<<"Content-Type is null!"<<endl;
        return E_Decode_Error;
    }
    return E_Suc;
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
    return E_Suc;
}



int Request::DecodeData()
{
    if(HEADER["Content-Type"] == "application/x-www-form-urlencoded")
    {
        vector<string> paras = Split(ToString(DATA.pointer), "&");
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
                return E_Decode_Error;
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
                // 切分数据部
                string delim_str = "\r\n--" + token[2].str();
                const char * delim = delim_str.c_str();
                vector<Buffer> parts = Split(DATA.pointer, DATA.length, delim, strlen(delim), false);

                char line_delim[] = "\r\n\r\n";
                for(unsigned int i=0; i<parts.size(); i++)
                {
                    int length = parts[i].length;
                    Byte * data = SplitBuffer(parts[i].pointer, length, line_delim, strlen(line_delim));
                    string form_header = ToString(parts[i].pointer);
                    regex data_reg("[\\s\\S]*Content-Disposition: form-data; name=\"(.*)\"(; filename=\"(.*)\"[\\s\\S]+?Content-Type: (.*))?$");
                    if(regex_match(form_header, token, data_reg))
                    {
                        // file
                        if(token[3] != "" && token[4] != "")
                        {
                            SaveDataToFile(token[3], data, length);
                        }
                        // form
                        else if(token[1] != "")
                        {
                            POST[token[1]] = ToString(data);
                        }
                    }
                    else
                    {
                        logger_<<ERROR<<"form_header decode error. form_header:"<<form_header<<endl;
                        return E_Decode_Error;
                    }
                }
            }
            else
            {
                logger_<<ERROR<<"unknown Content-Type:"<<HEADER["Content-Type"]<<endl;
                return E_Decode_Error;
            }
        }
        else
        {
            logger_<<ERROR<<"Content-Type decode error:"<<token[1]<<endl;
            return E_Decode_Error;
        }
    }
    return E_Suc;
}

int Request::DecodeHeader()
{
    if(HEADER["Accept-Encoding"].find("gzip") != string::npos)
    {
        META.COMPRESS = true;
    }

    if(HEADER["Connection"] == "close")
    {
        META.KEEP_ALIVE = false;
    }
    else if(HEADER["Connection"] == "keep-alive" || META.PROTOCOL == "HTTP/1.1")
    {
        META.KEEP_ALIVE = true;
    }

    META.ETAG = HEADER["If-None-Match"];

    return 0;
}


int Request::Reset()
{
    HEADER.clear();
    GET.clear();
    POST.clear();
    COOKIE.clear();
    memset(buffer_, 0, buffer_size_);
    header_length_ = 0;
    return E_Suc;
}


int Request::EnlargeBuffer(int new_size)
{
    logger_<<DEBUG<<"Enlarge buffer from ["<<buffer_size_<<"] to ["<<new_size<<"]"<<endl;
    if(buffer_size_ >= new_size)
    {
        logger_<<ERROR<<"The new size["<<new_size<<"] to be set can not smaller than the current size["<<buffer_size_<<"]"<<endl;
        return E_Operate_Error;
    }
    Byte * new_buffer = new Byte[new_size];
    memcpy(new_buffer, buffer_, buffer_size_);
    delete [] buffer_;
    buffer_ = new_buffer;
    buffer_size_ = new_size;
    return E_Suc;
}

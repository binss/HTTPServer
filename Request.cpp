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
    header_length_ = 0;
    buffer_size_ = REQUEST_DEFAULT_BUFFER_SIZE;
    buffer_length_ = 0;
    buffer_ = new char[buffer_size_];
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
        return -1;
    }
    DATA.length = length;
    char delim[] = "\r\n\r\n";
    DATA.pointer = SplitBuffer(buffer_, DATA.length, delim, strlen(delim));
    header_length_ = DATA.pointer - buffer_;
    if(DATA.pointer == NULL)
    {
        logger_<<ERROR<<"Part decode error!"<<endl;
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
            if(DATA.length != ToType<int, string>(HEADER["Content-Length"]))
            {
                logger_<<WARNING<<"The length of data["<<DATA.length<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
                return -100;
            }
            if(HEADER["Content-Type"] != "")
            {
                DecodeData();
            }
            else
            {
                logger_<<ERROR<<"Content-Type is null!"<<endl;
            }
        }


    }
    return 0;
}

int Request::Append(int new_length)
{
    DATA.pointer = buffer_ + header_length_;
    int data_length = new_length - header_length_;
    if(data_length != ToType<int, string>(HEADER["Content-Length"]))
    {
        logger_<<WARNING<<"The length of data["<<data_length<<"] is not equal to the Content-Length["<<HEADER["Content-Length"]<<"]!"<<endl;
        return -100;
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

int Request::SaveDataToFile(string filename, char * data, int data_length)
{

    string path = UPLOAD_DIR + filename;
    FILE * template_file = fopen(path.c_str(), "w");
    int offset = 0;
    while(offset < data_length)
    {
        int length = fwrite(data + offset, sizeof(char), data_length - offset, template_file);
        offset += length;
    }
    fclose(template_file);
    return 0;
}

int Request::DecodeData()
{
    if(HEADER["Content-Type"] == "application/x-www-form-urlencoded")
    {
        vector<string> paras = Split(string(DATA.pointer), "&");
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

                // 切分数据部
                string delim_str = "\r\n--" + token[2].str();
                const char * delim = delim_str.c_str();
                vector<Buffer> parts = Split(DATA.pointer, DATA.length, delim, strlen(delim), false);

                char line_delim[] = "\r\n\r\n";
                for(unsigned int i=0; i<parts.size(); i++)
                {
                    int length = parts[i].length;
                    char * data = SplitBuffer(parts[i].pointer, length, line_delim, strlen(line_delim));
                    string form_header(parts[i].pointer);
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
                            POST[token[1]] = string(data);
                        }
                    }
                    else
                    {
                        logger_<<ERROR<<"form_header decode error. form_header:"<<form_header<<endl;
                    }
                }
            }
            else
            {
                logger_<<ERROR<<"unknown Content-Type:"<<HEADER["Content-Type"]<<endl;
            }
        }
        else
        {
            logger_<<ERROR<<"Content-Type decode error:"<<token[1]<<endl;
        }
    }
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
    return 0;
}


int Request::EnlargeBuffer(int new_size)
{
    logger_<<DEBUG<<"Enlarge buffer from ["<<buffer_size_<<"] to ["<<new_size<<"]"<<endl;
    if(buffer_size_ >= new_size)
    {
        logger_<<ERROR<<"The new size["<<new_size<<"] to be set can not smaller than the current size["<<buffer_size_<<"]"<<endl;
        return -1;
    }
    char * new_buffer = new char[new_size];
    memcpy(new_buffer, buffer_, buffer_size_);
    // copy(buffer_, buffer_ + buffer_size_, new_buffer);
    delete [] buffer_;
    buffer_ = new_buffer;
    buffer_size_ = new_size;
    return 0;
}

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

#include "Response.h"


const string TEMPLATES_DIR = "/home/binss/HTTPServer/templates";
const string TEMPLATES_ERROR_DIR = "/home/binss/HTTPServer/templates/error";
const string RESOURCES_DIR = "/home/binss/HTTPServer/resources";

Response::Response()
{
    // 手动初始化，今后改成从文件等读入
    DATA_TYPES[""] = 0;
    DATA_TYPES[".js"] = 1;
    DATA_TYPES["css"] = 2;

    DATA_TYPES["png"] = 11;
    DATA_TYPES["jpg"] = 12;
    DATA_TYPES["gif"] = 13;

    buffer_length_ = 0;
    data_buffer_length_ = 0;
    memset(buffer_, 0, sizeof(buffer_));
    memset(data_buffer_, 0, sizeof(data_buffer_));
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

    protocol_ = request_header["protocol"];
    if(request_header["uri"] != "")
    {
        LoadData(request_header["uri"]);
    }

    return 0;
}


template<class TO, class TI>
inline TO ToType(const TI& input_obj)
{
    stringstream ss;
    ss << input_obj;

    TO output_obj;
    ss >> output_obj;

    return output_obj;
}


int Response::LoadData(string uri)
{
    if(uri != "")
    {
        if(uri.find("..") == string::npos)
        {
            string path;
            string file_type;
            FILE* template_file = NULL;
            if(uri.length() > 4)
            {
                file_type = uri.substr(uri.length() - 3);
                type_ = DATA_TYPES[file_type];
            }
            else
            {
                type_ = 0;
            }
            printf("[debug]file type: %s[%d]\n", file_type.c_str(), type_);
            switch(type_)
            {
                case 1:
                {
                    header_["Content-Type"] = "text/javascript";
                    break;
                }
                case 2:
                {
                    header_["Content-Type"] = "text/css";
                    break;
                }

                case 11:
                case 12:
                case 13:
                {
                    header_["Content-Type"] = "image/" + file_type;
                    break;
                }
                default:
                {
                    header_["Content-Type"] = "text/html";
                }
            }
            if(type_ < 10)
            {
                if(uri == "/")
                {
                    uri = "/index.html";
                    header_["Content-Type"] = "text/html";
                }
                path = TEMPLATES_DIR + uri;
                template_file = fopen(path.c_str(), "r");
            }
            else if(type_ > 10)
            {
                path = RESOURCES_DIR + uri;
                template_file = fopen(path.c_str(), "rb");

            }
            printf("[debug]file path:%s\n", path.c_str());
            if(template_file)
            {
                int length = fread(data_buffer_, sizeof(data_buffer_[0]), DATA_BUFFER_SIZE - 1, template_file);
                if(length > 0)
                {
                    data_buffer_length_ = length;
                }
                printf("[debug]data buffer len %d\n", data_buffer_length_);
                fclose(template_file);
            }

            if(protocol_ == "HTTP/1.1" && type_ == 0)
            {
                header_["Transfer-Encoding"] = "chunked";
            }
            if(type_ > 10 && type_ < 14)
            {
                header_["Content-Length"] = ToType<string, int>(data_buffer_length_);
                header_["Accept-Ranges"] = "bytes";
            }
        }
        else
        {
            // 禁止.. 防止目录外文件被返回 直接返回403
            // 403
        }
    }
    else
    {
        // 404
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
    if(type_ > 10 && type_ < 14)
    {
        printf("[debug]pic\n");
        memcpy(buffer_ + buffer_length_, data_buffer_, data_buffer_length_);
        buffer_length_ += data_buffer_length_;
    }
    else
    {
        char content_length[20];
        sprintf(content_length, "%x\r\n", data_buffer_length_);
        memcpy(buffer_ + buffer_length_, content_length, strlen(content_length));
        buffer_length_ += strlen(content_length);

        memcpy(buffer_ + buffer_length_, data_buffer_, data_buffer_length_);
        buffer_length_ += data_buffer_length_;

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
    printf("[debug]%d\n", buffer_length_);
    return buffer_length_;
}



int Response::Reset()
{
    header_.clear();
    buffer_length_ = 0;
    data_buffer_length_ = 0;
    memset(buffer_, 0, sizeof(buffer_));
    memset(data_buffer_, 0, sizeof(data_buffer_));
    return 0;
}

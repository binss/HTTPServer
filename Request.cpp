/***********************************************************
 * @FileName:      Request.cpp
 * @Author:        binss
 * @Create:        2015-09-13 20:59:10
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/


#include <iostream>
#include <stdlib.h>
#include <boost/algorithm/string/regex.hpp>
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
    vector<string> parts;
    vector<string> lines;
    boost::smatch token;
    // 先切出header和data
    split_regex(parts, str, regex( "\r\n\r\n" ));
    if(parts.size() != 2)
    {
        logger_<<ERROR<<"Part decode error! lenght:"<<parts.size()<<endl;
        return -2;
    }
    // 解析header
    split_regex(lines, parts[0], regex( "\r\n" ));
    if(lines.size() <= 0)
    {
        logger_<<ERROR<<"Header decode error! header:"<<parts[0]<<endl;
        return -3;
    }
    // 对于第一行作特殊处理
    {
        vector<string> metas;
        boost::split(metas, lines[0], boost::is_any_of(" "));
        if(metas.size() != 3)
        {
            logger_<<ERROR<<"Header meta decode error! meta:"<<lines[0]<<endl;
            return -2;
        }
        header_["method"] = metas[0];
        header_["uri"] = metas[1];
        header_["protocol"] = metas[2];
    }
    for (size_t i = 1; i < lines.size(); ++ i)
    {
        regex reg("(.*): (.*)");
        if ( boost::regex_search(lines[i], token, reg) )
        {
            if( token.size() == 3 )
            {
                header_[token[1]] = token[2];
            }
            else
            {
                logger_<<ERROR<<"Header line decode error! meta:"<<token[0]<<endl;
            }
        }
    }


    if(header_["method"] == "POST" && header_.find("Content-Length") != header_.end())
    {
        if(parts[1].length() != ToType<unsigned int, string>(header_["Content-Length"]))
        {
            logger_<<WARNING<<"The length of data["<<parts[1].length()<<"] is not equal to the Content-Length["<<header_["Content-Length"]<<"]!"<<endl;
        }
        data_ = parts[1];
    }

    // TODO
    // 处理cookie
    // 处理url参数
    // 处理If-Modified-Since -返回304
    //

    return 0;
}

int Request::Reset()
{
    header_.clear();
    data_.clear();
    memset(buffer_, 0, REQUEST_BUFFER_SIZE);
    return 0;
}

char * Request::GetBuffer()
{
    return buffer_;
}

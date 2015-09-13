/***********************************************************
 * @FileName:      RequestHandler.cpp
 * @Author:        binss
 * @Create:        2015-09-13 20:59:10
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/


#include "RequestHandler.h"
#include <iostream>
#include <stdlib.h>
#include <boost/algorithm/string/regex.hpp>

RequestHandler::RequestHandler()
{
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

int RequestHandler::ParseRequest(char buf[], int len)
{
    if( len <= 0 )
    {
        cout<<"request error! lenght:"<<len<<endl;
        return -1;
    }
    string str(buf);
    vector<string> parts;
    vector<string> lines;
    boost::smatch token;
    int data_line = 0;
    // 先切出header和data
    split_regex(parts, str, regex( "\r\n\r\n" ));
    if(parts.size() != 2)
    {
        cout<<"part decode error! lenght:"<<parts.size()<<" request:"<<str<<endl;
        return -2;
    }
    // 解析header
    split_regex(lines, parts[0], regex( "\r\n" ));
    if(lines.size() <= 0)
    {
        cout<<"header decode error!"<<endl;
        return -3;
    }
    // 对于第一行作特殊处理
    {
        vector<string> metas;
        boost::split(metas, lines[0], boost::is_any_of(" "));
        if(metas.size() != 3)
        {
            cout<<"header meta decode error!"<<endl;
            return -2;
        }
        header_["method"] = metas[0];
        header_["url"] = metas[1];
        header_["protocol"] = metas[2];
    }
    for (size_t i = 1; i < lines.size(); ++ i)
    {
        // cout << lines[i] << endl;
        regex reg("(.*): (.*)");
        if ( boost::regex_search(lines[i], token, reg) )
        {
            // cout << token.size() << std::endl;
            if( token.size() == 3 )
            {
                header_[token[1]] = token[2];
            }
            else
            {
                cout<<"header line decode error: "<<token[0]<<endl;
            }
        }
    }

    if(header_["method"] == "POST" && header_.find("Content-Length") != header_.end())
    {
        if(parts[1].length() != ToType<int, string>(header_["Content-Length"]))
        {
            cout<<"warning: The length of data is not equal to the Content-Length!"<<endl;
        }
        data_ = parts[1];
        // cout<<"data: "<<data_<<endl;
    }

    return 0;
}




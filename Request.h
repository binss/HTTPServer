/***********************************************************
 * @FileName:      Request.h
 * @Author:        binss
 * @Create:        2015-09-13 20:58:56
 * @Description:
 * @History:
    <author>    <time>    <version>    <desc>
***********************************************************/

#ifndef  __REQUEST_H__
#define  __REQUEST_H__

#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <regex>
#include "Logger.h"

using namespace std;


class Request
{
public:
    Request();
    ~Request();
    int Parse(int length);
    int Reset();
    char * GetBuffer() { return buffer_; }
    int GetBufferSize() { return buffer_size_; }
    int EnlargeBuffer(int new_size);
    int Append(int new_length);

private:
    int DecodeCookie(string cookie_str);
    int DecodeData();
    int SaveDataToFile(string filename, char * data, int data_length);

public:
    unordered_map<string, string> GET;
    unordered_map<string, string> POST;
    unordered_map<string, string> HEADER;
    unordered_map<string, string> COOKIE;
    string URI;
    string RAW_URI;
    string METHOD;
    string PROTOCOL;
    Buffer DATA;

private:
    char * buffer_;
    int buffer_length_;
    int buffer_size_;
    int header_length_;
    Logger logger_;
};

#endif

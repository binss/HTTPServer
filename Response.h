/***********************************************************
* FileName:      Response.h
* Author:        binss
* Create:        2015-10-29 09:19:53
* Description:   No Description
***********************************************************/


#ifndef  __RESPONSE_H__
#define  __RESPONSE_H__

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#define BUFFER_SIZE 1024 * 1024
#define DATA_BUFFER_SIZE 1024 * 1000

class Response
{
public:
    Response();
    int Init(unordered_map<string, string> &request_header);
    int GetTime(char * time, int length);
    int Build();
    string & GetStr();
    int Reset();
    int LoadData(string uri);
    char * GetBuffer();
    int GetBufferLength();

    unordered_map<string, string> header_;
    char buffer_[BUFFER_SIZE];
    int buffer_length_;
    char data_buffer_[DATA_BUFFER_SIZE];
    int data_buffer_length_;
    string response_str_;
    int type_;
    string protocol_;
    map<string, int> DATA_TYPES;

};

#endif

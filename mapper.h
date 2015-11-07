/***********************************************************
* FileName:      Mapper.h
* Author:        binss
* Create:        2015-11-03 22:12:36
* Description:   简单的单例工具类，不考虑线程安全等
***********************************************************/

#ifndef  __MAPPER_H__
#define  __MAPPER_H__

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;


class Mapper
{
private:
    Mapper();


public:
    static Mapper * GetInstance();
    void InitContentTypeMap();
    void InitURIMap();

    int GetContentType(string file_type);
    string GetURI(string request_uri, int content_type);

private:
    static Mapper *mapper;
    unordered_map<string, int> content_type_map_;
    unordered_map<string, string> uri_map_;

};

#endif


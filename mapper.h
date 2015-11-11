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
#include "View.h"
#include "Constants.h"

using namespace std;


class Mapper
{
private:
    Mapper();


public:
    static Mapper * GetInstance();
    void InitContentTypeMap();
    void InitURIMap();
    void InitViewMap();
    void InitReasonMap();

    int GetContentType(string file_type);
    string GetURI(int code);
    View GetView(string request_uri);
    string & GetReason(int code);

private:
    static Mapper *mapper;
    unordered_map<string, int> content_type_map_;
    unordered_map<int, string> uri_map_;
    unordered_map<string, View> view_map_;
    unordered_map<int, string> reason_map_;

};

#endif


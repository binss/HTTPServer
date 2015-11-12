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
#include "Logger.h"

using namespace std;


class Mapper
{
public:
    static Mapper * GetInstance();
    int GetContentType(string file_type);
    View GetView(string target);
    string & GetReason(int code);

private:
    Mapper();
    void InitContentTypeMap();
    void InitViewMap();
    void InitReasonMap();

private:
    static Mapper *mapper;
    Logger logger_;

    unordered_map<string, int> content_type_map_;
    unordered_map<string, View> view_map_;
    unordered_map<int, string> reason_map_;

};

#endif


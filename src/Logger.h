/***********************************************************
* FileName:      Logger.h
* Author:        binss
* Create:        2015-11-08 19:41:02
* Description:   向终端打印日志/写入日志文件
***********************************************************/

#ifndef __LOGGER_H__
#define __LOGGER_H__


#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Constants.h"
#include "GlobalUtil.h"

using namespace std;

class Logger
{
public:
    Logger();
    Logger(string name, LogLevel level, bool persist);
    ~Logger();
    void SetLevel(const LogLevel& level);
    void SetLineLevel(const LogLevel& level);
    void Print();
    // Logger& operator=(const Logger &logger);

    Logger& operator<<(long unsigned int content);
    Logger& operator<<(const LogLevel& level);
    Logger& operator<<(const char * content);
    Logger& operator<<(string content);
    Logger& operator<<(int content);
    Logger& operator<<(unsigned int content);
    Logger& operator<<(unordered_map<string, string> &content);
    Logger& operator<<(Logger& (*fun) (Logger&));

private:
    string name_;
    LogLevel level_;
    bool persist_;
    ofstream file_;
    stringstream stream_;
    LogLevel line_level_;
};

namespace std
{
    inline Logger& endl(Logger& logger)
    {
        logger<<"\n";
        logger.Print();
        return logger;
    }
}

#endif

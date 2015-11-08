/***********************************************************
* FileName:      Logger.h
* Author:        binss
* Create:        2015-11-08 19:41:02
* Description:   向终端打印日志/写入日志文件
***********************************************************/

#ifndef __LOGGER__
#define __LOGGER__

#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "Constants.h"

using namespace std;

class Logger
{
public:
    Logger(const char * name, LogLevel level, bool persist);
    ~Logger();
    void SetLevel(const LogLevel& level);
    void SetLineLevel(const LogLevel& level);
    void Print();

    Logger& operator<<(const LogLevel& level);
    Logger& operator<<(const char * content);
    Logger& operator<<(int content);
    Logger& operator<<(unsigned int content);
    Logger& operator<<(Logger& (*fun) (Logger&));

private:
    const char * name_;
    LogLevel level_;
    bool persist_;
    ofstream file_;
    stringstream stream_;
    LogLevel line_level_;

    // ostream ostream_;

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

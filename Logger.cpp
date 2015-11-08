/***********************************************************
* FileName:      Logger.cpp
* Author:        binss
* Create:        2015-11-08 19:48:10
* Description:   No Description
***********************************************************/

#include "Logger.h"

Logger::Logger(const char * name, LogLevel level, bool persist):name_(name), level_(level), persist_(persist)
{
    if(persist_)
    {
    }
}

Logger::~Logger()
{
    cout<<"\033[0m";
}


void Logger::SetLevel(const LogLevel& level)
{
    level_ = level;
}

Logger& Logger::operator<<(const LogLevel& level)
{
    SetLineLevel(level);
    stream_<<"("<<name_<<") ";
    return *this;
}

Logger& Logger::operator<<(const char * content)
{
    stream_<<content;
    return *this;
}

Logger& Logger::operator<<(int content)
{
    stream_<<content;
    return *this;
}

Logger& Logger::operator<<(unsigned int content)
{
    stream_<<content;
    return *this;
}

Logger& Logger::operator<<(Logger& (*fun) (Logger&))
{
    return (*fun)(*this);
}

void Logger::Print()
{
    cout<<stream_.str();
    stream_.str("");
}

void Logger::SetLineLevel(const LogLevel& level)
{
    line_level_ = level;
    switch(level)
    {
        case VERBOSE:
        {
            stream_<<"[VERBOSE] ";
            cout<<"\033[37m";
            break;
        }
        case DEBUG:
        {
            stream_<<"[DEBUG] ";
            cout<<"\033[36m";
            break;
        }
        case INFO:
        {
            stream_<<"[INFO] ";
            cout<<"\033[32m";
            break;
        }
        case WARNING:
        {
            stream_<<"[WARNING] ";
            cout<<"\033[33m";
            break;
        }
        case ERROR:
        {
            stream_<<"[ERROR] ";
            cout<<"\033[31m";
            break;
        }
        case CRITICAL:
        {
            stream_<<"[CRITICAL] ";
            cout<<"\033[1;31m";
            break;
        }
        default:
        {
            cout<<"\033[0m\n";
        }
        // cout<<"("<<name_<<")"<<content<<"\033[0m"<<endl;
    }
}


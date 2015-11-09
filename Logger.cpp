/***********************************************************
* FileName:      Logger.cpp
* Author:        binss
* Create:        2015-11-08 19:48:10
* Description:   No Description
***********************************************************/

#include "Logger.h"

Logger::Logger()
{

}

Logger::Logger(string name, LogLevel level, bool persist):name_(name), level_(level), persist_(persist)
{
    if(persist_)
    {
        EnsureDirectory(LOG_FILE_PATH);
        string path = LOG_FILE_PATH + GetCurFormatTime("%Y%m%d") + ".log";
        file_.open(path, ios::app);
        if(!file_.is_open())
        {
            cout<<"\033[31m[ERROR] (Logger) Open log file failed! Path:"<<path<<endl;
        }
    }
}

// Logger& Logger::operator=(const Logger &logger)
// {
//     if( this == &logger )
//     {
//         return *this;
//     }
//     name_ = logger.name_;
//     level_ = logger.level_;
//     persist_ = logger.persist_;
//     return *this;
// }

Logger::~Logger()
{
    cout<<"\033[0m";
    if(file_.is_open())
    {
        file_.flush();
        file_.close();
    }
}


void Logger::SetLevel(const LogLevel& level)
{
    level_ = level;
}

Logger& Logger::operator<<(const LogLevel& level)
{
    stream_<<"["<<GetCurFormatTime("%F %T")<<"]";
    SetLineLevel(level);
    stream_<<"["<<name_<<"]  ";
    return *this;
}

Logger& Logger::operator<<(const char * content)
{
    stream_<<content;
    return *this;
}

Logger& Logger::operator<<(string content)
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

Logger& Logger::operator<<(long unsigned int content)
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
    if(line_level_ >= level_)
    {
        cout<<stream_.str();
        cout.flush();
    }
    if(persist_ && file_.is_open())
    {
        file_<<stream_.str();
        file_.flush();
    }
    stream_.str("");
}

void Logger::SetLineLevel(const LogLevel& level)
{
    line_level_ = level;
    switch(level)
    {
        case VERBOSE:
        {
            stream_<<"[VERBOSE]";
            cout<<"\033[37m";
            break;
        }
        case DEBUG:
        {
            stream_<<"[DEBUG]";
            cout<<"\033[36m";
            break;
        }
        case INFO:
        {
            stream_<<"[INFO]";
            cout<<"\033[32m";
            break;
        }
        case WARNING:
        {
            stream_<<"[WARNING]";
            cout<<"\033[33m";
            break;
        }
        case ERROR:
        {
            stream_<<"[ERROR]";
            cout<<"\033[31m";
            break;
        }
        case CRITICAL:
        {
            stream_<<"[CRITICAL]";
            cout<<"\033[1;31m";
            break;
        }
        default:
        {
            cout<<"\033[0m\n";
        }
    }
}


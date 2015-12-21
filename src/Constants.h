/***********************************************************
* FileName:      Constants.h
* Author:        binss
* Create:        2015-11-08 19:51:10
* Description:   No Description
***********************************************************/

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <string>
#include <unordered_map>

using namespace std;

#define TIME_BUFFER_LEN 50
#define SERV_PORT 8888
#define LISTENQ 1024    /* 2nd argument to listen() */
#define EPOLL_SIZE 1024
#define EPOLL_TIMEOUT 500
#define REQUEST_DEFAULT_BUFFER_SIZE 1024

#define NORMAL_BUFFER_SIZE 1024 * 1024 + 2048
#define BIG_BUFFER_SIZE 1024 * 1024 * 10 + 2048

#define COMPRESS_BUFFER_ADD_SIZE 1024

#define LOG_FILE_PATH "/home/binss/HTTPServer/log/"

#define TEMPLATES_DIR "/home/binss/HTTPServer/templates"
#define RESOURCES_DIR "/home/binss/HTTPServer/resources"
#define UPLOAD_DIR "/home/binss/HTTPServer/upload/"

#define COMPRESS_ON true
#define COMPRESS_LEVEL 9

#define MAX_AGE 6000

#define Connection_Alive_Time 60

enum LogLevel
{
    VERBOSE = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum ReturnCode
{
    E_Suc = 0,

    E_Decode_Error = -10,
    E_Operate_Error = -11,
    E_Request_Not_Complete = -100,
    E_Client_Close = -101,
    E_Server_Close = -102,
    E_Timer_Error = -103,
    E_Buffer_Error = -104,
};


struct Meta
{
    string URI;
    string RAW_URI;
    string METHOD;
    string PROTOCOL;
    string ETAG;
    bool COMPRESS;
    bool KEEP_ALIVE;
};

typedef unordered_map<string, string> SSMap;

#define DB_HOST "tcp://172.17.0.2:3306"
#define DB_USER "binss"
#define DB_PASSWORD "123456"
#define DB_NAME "dudu"


enum FieldType
{
    INT = 1,
    STRING = 2,
    DOUBLE = 3,
    BOOLEAN = 4,
};

typedef unordered_map<string, string> FilterMap;
typedef unordered_map<string, string> OrderMap;

#endif

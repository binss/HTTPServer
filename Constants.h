/***********************************************************
* FileName:      Constants.h
* Author:        binss
* Create:        2015-11-08 19:51:10
* Description:   No Description
***********************************************************/

#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

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
#define UPLOAD_DIR "/home/binss/HTTPServer/upload"

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

enum ErrorNo
{
    E_Request_Not_Complete = -100,
    E_Client_Close = -101,
};

#endif

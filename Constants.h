/***********************************************************
* FileName:      Constants.h
* Author:        binss
* Create:        2015-11-08 19:51:10
* Description:   No Description
***********************************************************/

#ifndef __CONSTANTS__
#define __CONSTANTS__


const char LOG_FILE_PATH[50] = "/home/binss/HTTPServer/log/";


enum LogLevel
{
  VERBOSE = 0,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  CRITICAL
};


#endif

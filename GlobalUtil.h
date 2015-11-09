/***********************************************************
* FileName:      GlobalUtil.h
* Author:        binss
* Create:        2015-11-07 17:57:16
* Description:   No Description
***********************************************************/

#ifndef  __GLOBAL_UTIL_H__
#define  __GLOBAL_UTIL_H__

#include <time.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

string GetTime(int offset);
string GetCurFormatTime(string format);
void EnsureDirectory(const char * path);
#endif

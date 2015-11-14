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
#include <zlib.h>
#include <vector>
#include <algorithm>

using namespace std;

string GetTime(int offset);
string GetCurFormatTime(string format);
void EnsureDirectory(const char * path);
int Compress(unsigned char *dest, uLong & dest_len, unsigned char *src, uLong src_len, int level);
vector<string> split(const string& s, const string& delim, const bool keep_empty = true);

#endif

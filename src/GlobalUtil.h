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
#include <cstring>
#include <sstream>

using namespace std;

struct Buffer
{
    Byte * pointer;
    int length;
};

string GetTime(int offset);
string GetCurFormatTime(string format);

void EnsureDirectory(const char * path);
int SaveDataToFile(string filename, Byte * data, int data_length);

int Compress(Byte *dest, uLong & dest_len, Byte *src, uLong src_len, int level);
vector<string> Split(const string& s, const string& delim, const bool keep_empty = true);
vector<Buffer> Split(const Byte * buffer, int buffer_length, const char * delim, int delim_length, const bool keep_empty);
Byte * SplitBuffer(Byte * buffer, int & buffer_length, const char * delim, int delim_length);

template<class TO, class TI>
inline TO ToType(const TI& input_obj)
{
    stringstream ss;
    ss << input_obj;
    TO output_obj;
    ss >> output_obj;
    return output_obj;
}


inline string ToString(Byte * input_obj)
{
    return string((char*)input_obj);
}

#endif

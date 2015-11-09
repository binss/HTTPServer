/***********************************************************
* FileName:      GlobalUtil.cpp
* Author:        binss
* Create:        2015-11-07 17:57:23
* Description:   No Description
***********************************************************/


#include "GlobalUtil.h"
#include "Constants.h"

string GetTime(int offset)
{
    char time_buf[TIME_BUFFER_LEN];
    time_t now_time;
    time(&now_time);
    time_t target_time = now_time + offset;
    struct tm *time_ptr = localtime(&target_time);
    strftime(time_buf, TIME_BUFFER_LEN, "%a, %d %b %T %Y GMT", time_ptr);
    return string(time_buf);
}

string GetCurFormatTime(string format)
{
    char time_buf[TIME_BUFFER_LEN];
    time_t now_time;
    time(&now_time);
    struct tm *time_ptr = localtime(&now_time);
    strftime(time_buf, TIME_BUFFER_LEN, format.c_str(), time_ptr);
    return string(time_buf);
}

void EnsureDirectory(const char * path)
{
    struct stat st = {0};
    if(stat(path, &st) == -1)
    {
        mkdir(path, 0700);
    }
}

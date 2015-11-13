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


int Compress(unsigned char *dest, uLong & dest_len, unsigned char *src, uLong src_len, int level)
{
    int ret;
    z_stream strm;
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, level, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    // deflateInit(&strm, level);
    if (ret != Z_OK)
    {
        return ret;
    }

    strm.next_in = src;
    strm.avail_in = src_len;

    // flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;

    strm.next_out  = dest;
    strm.avail_out = dest_len;

    ret = deflate(&strm, Z_FINISH);
    dest_len = dest_len - strm.avail_out;

    (void)deflateEnd(&strm);
}


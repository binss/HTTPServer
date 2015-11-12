/***********************************************************
* FileName:      CacheManager.h
* Author:        binss
* Create:        2015-11-07 18:18:40
* Description:   No Description
***********************************************************/

#ifndef  __CACHE_MANAGER_H__
#define  __CACHE_MANAGER_H__

#define BIG_DATA_SIZE 1024 * 1024 * 10

#include <string>
#include <unordered_map>
#include <cstring>

#include "Logger.h"

using namespace std;

class Cache
{
public:
    Cache()
    {
        hit_time_ = 0;
    }

public:
    int type_;
    int size_;
    int hit_time_;
    char *data_;
};

class CacheManager
{
public:
    static CacheManager * GetInstance();
    CacheManager();
    int Init();
    Cache *GetCache(string uri, int type);
    int CleanBuffer();
    int ShowCachesStatus();


private:
    static CacheManager *manager;
    bool IsInit_;
    unordered_map<string, Cache *> caches_;
    char buffer_[BIG_DATA_SIZE];
    Logger logger_;

};


#endif




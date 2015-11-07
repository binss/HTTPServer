/***********************************************************
* FileName:      CacheManager.cpp
* Author:        binss
* Create:        2015-11-07 18:28:50
* Description:   No Description
***********************************************************/

#include "CacheManager.h"

CacheManager * CacheManager::manager = NULL;

CacheManager * CacheManager::GetInstance()
{
    if(manager == NULL)
    {
        manager = new CacheManager();
    }
    return manager;
}

CacheManager::CacheManager()
{

}

Cache * CacheManager::GetCache(string path, int type)
{
    if(path == "")
    {
        return NULL;
    }
    Cache *cache = caches_[path];
    if(NULL == cache)
    {
        // cache miss
        FILE * template_file = fopen(path.c_str(), "r");
        if( NULL == template_file)
        {
            printf("[error]can not find file: %s\n", path.c_str());
            return NULL;
        }
        CleanBuffer();
        int length = fread(buffer_, sizeof(char), BIG_DATA_SIZE - 1, template_file);
        if(length > 0)
        {
            cache = new Cache();
            cache->type_ = type;
            cache->size_ = length;
            cache->data_ = new char[length];
            memcpy(cache->data_, buffer_ , length);
            caches_[path] = cache;
        }
        // printf("[debug]data buffer len %d\n", data_buffer_length_);
        fclose(template_file);
    }
    else
    {
        // cache hit
        cache->hit_time_ ++;
        printf("[debug]hit cache, size:%d\n", cache->size_);
    }
    return cache;

}

int CacheManager::CleanBuffer()
{
    memset(buffer_, 0, BIG_DATA_SIZE);
    return 0;
}

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

CacheManager::CacheManager():logger_("CacheManager", DEBUG, true)
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
        if(type == 1)
        {
            path = TEMPLATES_DIR + path;
        }
        else
        {
            path = RESOURCES_DIR + path;
        }
        logger_<<DEBUG<<"path:"<<path<<endl;
        FILE * template_file = fopen(path.c_str(), "r");
        if( NULL == template_file)
        {
            logger_<<ERROR<<"Can not find file: "<<path<<endl;
            return NULL;
        }
        // generate etag
        CleanBuffer();
        int length = fread(buffer_, sizeof(unsigned char), BIG_DATA_SIZE - 1, template_file);
        if(length > 0)
        {
            cache = new Cache();
            cache->type_ = type;
            cache->size_ = length;
            cache->SetETag(path);
            cache->data_ = new unsigned char[length];
            memcpy(cache->data_, buffer_ , length);
            // compressable 只压缩文本(type<20)
            if(COMPRESS_ON && cache->type_ < 20)
            {
                if(COMPRESS_LEVEL > 0 && COMPRESS_LEVEL < 10)
                {
                    // 多分配COMPRESS_BUFFER_ADD_SIZE，防止越压越大
                    cache->compress_data_ = new unsigned char[cache->size_ + COMPRESS_BUFFER_ADD_SIZE];
                    cache->compress_size_ = cache->size_ + COMPRESS_BUFFER_ADD_SIZE;
                    int ret = Compress(cache->compress_data_, cache->compress_size_, cache->data_, cache->size_, COMPRESS_LEVEL);
                    if( 0 == ret )
                    {
                        logger_<<DEBUG<<"Compress succeed. Size["<<cache->size_<<"]->["<<cache->compress_size_<<"]"<<endl;
                    }
                    else
                    {
                        delete []cache->compress_data_;
                        cache->compress_data_ = NULL;
                        cache->compress_size_ = 0;
                        logger_<<ERROR<<"Compress failed. Ret: "<<ret<<endl;
                    }
                }
                else
                {
                    logger_<<ERROR<<"Compress level should be 1-9, current["<<COMPRESS_LEVEL<<"]"<<endl;
                }
            }
            caches_[path] = cache;
        }
        fclose(template_file);
    }
    else
    {
        // cache hit
        cache->hit_time_ ++;
        logger_<<DEBUG<<"Hit cache: size:"<<cache->size_<<endl;
    }
    return cache;
}

int CacheManager::CleanBuffer()
{
    memset(buffer_, 0, BIG_DATA_SIZE);
    return 0;
}

void Cache::SetETag(string & path)
{
    struct stat st_buf;
    stat(path.c_str(), &st_buf);
    char buf[30];
    sprintf(buf, "%lx-%lx-%lx", st_buf.st_ino, st_buf.st_size, st_buf.st_mtime);
    this->etag_ = string(buf);
}

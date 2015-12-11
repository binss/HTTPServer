/***********************************************************
* FileName:      Response.cpp
* Author:        binss
* Create:        2015-10-29 09:19:33
* Description:   No Description
***********************************************************/

#include "GlobalUtil.h"
#include "Mapper.h"
#include "Response.h"

Response::Response():logger_("Response", DEBUG, true)
{
    raw_ = NULL;
    data_ = NULL;
    buffer_ = NULL;
    buffer_length_ = 0;
    buffer_size_ = 0;

    TYPE = 0;
    CODE = 0;
    meta_ = NULL;
    request_header_ = NULL;
    request_cookie_ = NULL;
}

Response::~Response()
{
    delete [] buffer_;
    buffer_ = NULL;
    delete [] raw_;
    raw_ = NULL;
    header_.clear();
}

int Response::Init(SSMap *header, SSMap *cookie, Meta *meta)
{
    request_header_ = header;
    request_cookie_ = cookie;
    meta_ = meta;

    DecodeTarget();

    // 接收范围请求
    // header_["Accept-Ranges"] = "bytes";
    AllocBuffer();

    return 0;
}

int Response::AllocBuffer()
{
    // 分配内存
    if(TYPE < 20)
    {
        if(buffer_ == NULL)
        {
            buffer_ = new Byte[NORMAL_BUFFER_SIZE];
            buffer_size_ = NORMAL_BUFFER_SIZE;
        }
    }
    if(TYPE >= 20)
    {
        if(buffer_ == NULL || buffer_size_ < BIG_BUFFER_SIZE)
        {
            delete []buffer_;
            buffer_ = new Byte[BIG_BUFFER_SIZE];
            buffer_size_ = BIG_BUFFER_SIZE;
        }
    }
    return 0;
}

int Response::DecodeTarget()
{
    string & uri = meta_->URI;
    string file_type = "default";
    if(uri != "")
    {
        if(uri.find("..") == string::npos)
        {

            if(uri.length() > 4)
            {
                file_type = uri.substr(uri.length() - 3);
                TYPE = Mapper::GetInstance()->GetContentType(file_type);
                if(TYPE > 0)
                {
                    // 请求资源
                    if( LoadCache(uri, TYPE) != 0 )
                    {
                        TYPE = 0;
                        target_ = "/404/";
                    }
                }
                else
                {
                    // 请求view
                    target_ = uri;
                }
            }
            else
            {
                target_ = uri;
            }
        }
        else
        {
            // 禁止.. 防止目录外文件被返回 直接返回403
            target_ = "/403/";
        }
    }
    else
    {
        target_ = "/404/";
    }

    logger_<<DEBUG<<"URI: "<<uri<<" target: "<<target_<<" File type: "<<file_type<<"["<<TYPE<<"]"<<endl;

    return 0;
}

int Response::BuildHeader()
{
    header_["Server"] = "Dudu Server/0.1";
    header_["Date"] = GetTime(0);

    switch(TYPE)
    {
        case 0:
        case 1:
        {
            header_["Content-Type"] = "text/html; charset=UTF-8"; break;
        }
        case 10:
        {
            header_["Content-Type"] = "text/javascript; charset=UTF-8"; break;
        }
        case 11:
        {
            header_["Content-Type"] = "text/css; charset=UTF-8"; break;
        }
        case 20:
        {
            header_["Content-Type"] = "image/png"; break;
        }
        case 21:
        {
            header_["Content-Type"] = "image/jpg"; break;
        }
        case 22:
        {
            header_["Content-Type"] = "image/gif"; break;
        }
        case 23:
        {
            header_["Content-Type"] = "image/ico"; break;
        }
        default:
        {
            logger_<<ERROR<<"Can not recognize type["<<TYPE<<"], set to default type[0]"<<endl;
            header_["Content-Type"] = "text/html; charset=UTF-8";
        }
    }

    if(meta_->PROTOCOL == "HTTP/1.0")
    {
        if(meta_->KEEP_ALIVE)
        {
            header_["Connection"] = "keep-alive";
        }
        if(meta_->COMPRESS)
        {
            header_["Content-Encoding"] = "gzip";
        }
        if(TYPE == 1)
        {
            // 立即过期，不缓存
            header_["Expires"] = "-1";
        }
        else
        {
            header_["Expires"] = GetTime(MAX_AGE);
        }

        header_["Content-Length"] = ToType<string, uLong>(size_);
    }
    else if(meta_->PROTOCOL == "HTTP/1.1")
    {
        if(!meta_->KEEP_ALIVE)
        {
            header_["Connection"] = "close";
        }

        // 对于页面，分chunked发送 TODO：分chucked
        if(TYPE == 1)
        {
            header_["Transfer-Encoding"] = "chunked";
            if(meta_->COMPRESS)
            {
                header_["Content-Encoding"] = "gzip";
            }
            // 不缓存页面
            header_["Cache-Control"] = "no-cache";
        }
        else
        {
            if(meta_->COMPRESS)
            {
                header_["Content-Encoding"] = "gzip";
            }
            // 设置cache时间
            if(MAX_AGE > 0)
            {
                header_["Cache-Control"] = "max-age=" + ToType<string, int>(MAX_AGE);
            }
            else
            {
                header_["Cache-Control"] = "no-cache";
            }

            // 判断etag
            if(meta_->ETAG != "" && meta_->ETAG == header_["ETag"])
            {
                header_["Content-Length"] = "0";
                CODE = 304;
            }
            else
            {
                header_["Content-Length"] = ToType<string, uLong>(size_);
            }

        }
    }

    return 0;
}

int Response::Build()
{
    if( CODE == 0 )
    {
        logger_<<WARNING<<"The response code should be set!"<<endl;
        CODE = 500;
        meta_->KEEP_ALIVE = false;
    }

    if( TYPE == 0 )
    {
        logger_<<WARNING<<"The type should be set!"<<endl;
        CODE = 500;
        meta_->KEEP_ALIVE = false;
    }

    if(NULL == data_)
    {
        logger_<<WARNING<<"The response data should be set!"<<endl;
        CODE = 500;
        meta_->KEEP_ALIVE = false;
    }
    else
    {
        BuildHeader();
    }


    // 填充header
    string reason = Mapper::GetInstance()->GetReason(CODE);
    string protocol = "HTTP/1.1";
    string header_str = protocol + " " + reason + "\r\n";
    for(unordered_map<string, string>::iterator iter = header_.begin(); iter != header_.end(); ++ iter)
    {
        header_str += (*iter).first + ": " + (*iter).second + "\r\n";
    }
    header_str += "\r\n";

    memcpy(buffer_, header_str.c_str(), header_str.length());
    buffer_length_ += header_str.length();

    if( CODE == 304 || CODE == 500 )
    {
        return 0;
    }


    // 填充data
    if( TYPE == 1 )
    {
        char content_length[20];

        sprintf(content_length, "%lx\r\n", size_);
        memcpy(buffer_ + buffer_length_, content_length, strlen(content_length));
        buffer_length_ += strlen(content_length);

        memcpy(buffer_ + buffer_length_, data_, size_);
        buffer_length_ += size_;


        char buffer_end[20] = "\r\n0\r\n\r\n";
        memcpy(buffer_ + buffer_length_, buffer_end, strlen(buffer_end));
        buffer_length_ += strlen(buffer_end);
    }
    else
    {
        memcpy(buffer_ + buffer_length_, data_, size_);
        buffer_length_ += size_;
    }
    return 0;
}

int Response::Reset()
{
    TYPE = 0;
    CODE = 0;
    target_ = "";
    header_.clear();
    buffer_length_ = 0;
    memset(buffer_, 0, buffer_size_);
    return 0;
}


int Response::LoadCache(string & path, int type)
{
    Cache * cache = CacheManager::GetInstance()->GetCache(path, type);
    if(NULL == cache)
    {
        logger_<<ERROR<<"Get cache instance error!"<<endl;
        return -1;
    }
    else
    {
        if(COMPRESS_ON && meta_->COMPRESS && cache->compress_size_ > 0 && cache->compress_data_ != NULL)
        {
            meta_->COMPRESS = true;
            size_ = cache->compress_size_;
            data_ = cache->compress_data_;
        }
        else
        {
            meta_->COMPRESS = false;
            size_ = cache->size_;
            data_ = cache->data_;
        }
        header_["ETag"] = cache->etag_;
    }
    return 0;
}

// ------------------------------------------
// ----------- User interfaces ---------------
// ------------------------------------------

int Response::SetCookie(const char *name, const char *value, string expires, const char *domain, const char *path, bool secure)
{
    stringstream buffer;
    buffer<<name<<"="<<value<<"; "<<"expires="<<expires<<"; ";
    if( NULL != domain )
    {
        buffer<<"domain="<<domain<<"; ";
    }
    if( NULL != path)
    {
        buffer<<"path="<<path<<"; ";
    }
    if( secure )
    {
        buffer<<"secure; ";
    }
    string key = "Set-Cookie";
    while( "" != header_[key])
    {
        key += " ";
    }
    header_[key] = buffer.str();
    return 0;
}

void Response::SetFile(string path)
{
    if(path.length() > 4)
    {
        path = "/" + path;
        string file_type = path.substr(path.length() - 3);
        TYPE = Mapper::GetInstance()->GetContentType(file_type);
        if(TYPE > 0)
        {
            LoadCache(path, TYPE);
        }
        else
        {
            logger_<<ERROR<<"SetFile failed. Can not recognize type["<<file_type<<"]"<<endl;
        }
    }
    else
    {
        logger_<<ERROR<<"SetFile failed. path["<<path<<"] error"<<endl;
    }
}

void Response::SetRawString(string str)
{
    TYPE = 1;
    meta_->COMPRESS = false;
    size_ = str.size();
    raw_ = new Byte[size_];
    memcpy(raw_, str.c_str() , size_);
    data_ = raw_;
}


void Response::SetCode(int code)
{
    CODE = code;
}

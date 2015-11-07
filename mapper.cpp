/***********************************************************
* FileName:      Mapper.cpp
* Author:        binss
* Create:        2015-11-06 11:24:22
* Description:   No Description
***********************************************************/

#include "Mapper.h"

const string TEMPLATES_DIR = "/home/binss/HTTPServer/templates";
const string RESOURCES_DIR = "/home/binss/HTTPServer/resources";

Mapper * Mapper::mapper = NULL;

Mapper * Mapper::GetInstance()
{
    if(mapper == NULL)
    {
        mapper = new Mapper();
    }
    return mapper;
}


Mapper::Mapper()
{
    InitContentTypeMap();
    InitURIMap();
}

void Mapper::InitContentTypeMap()
{
    // 0 - 9 chucked
    content_type_map_[""] = 0;
    content_type_map_["tml"] = 1;
    // 10 - 19 fixed-length text file
    content_type_map_[".js"] = 10;
    content_type_map_["css"] = 11;

    // 20 - 29 image
    content_type_map_["png"] = 20;
    content_type_map_["jpg"] = 21;
    content_type_map_["gif"] = 22;
}

void Mapper::InitURIMap()
{
    uri_map_["/"] = "/index.html";
    uri_map_["/403/"] = "/error/403.html";
    uri_map_["/404/"] = "/error/404.html";
}

int Mapper::GetContentType(string file_type)
{
    // 默认为0
    return content_type_map_[file_type];
}

string Mapper::GetURI(string request_uri, int content_type)
{
    // 默认为0
    string uri = uri_map_[request_uri];
    if(uri == "")
    {
        uri = request_uri;
    }
    if(content_type < 10)
    {
        return TEMPLATES_DIR + uri;
    }
    else
    {
        return RESOURCES_DIR + uri;
    }
}

/***********************************************************
* FileName:      Mapper.cpp
* Author:        binss
* Create:        2015-11-06 11:24:22
* Description:   No Description
***********************************************************/

#include "Mapper.h"

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
    InitViewMap();
    InitReasonMap();
}

void Mapper::InitContentTypeMap()
{
    // 0 - 9 chucked
    content_type_map_[""] = 0;
    content_type_map_["tml"] = 0;
    // 10 - 19 fixed-length text file
    content_type_map_[".js"] = 10;
    content_type_map_["css"] = 11;

    // 20 - 29 image
    content_type_map_["png"] = 20;
    content_type_map_["jpg"] = 21;
    content_type_map_["gif"] = 22;
    content_type_map_["ico"] = 23;
}

void Mapper::InitURIMap()
{
    // uri_map_["/"] = "/index.html";
    uri_map_[403] = "/error/403.html";
    uri_map_[404] = "/error/404.html";
}

void Mapper::InitReasonMap()
{
    // 0 - 9 chucked
    reason_map_[200] = "200 OK";
    reason_map_[403] = "403 Forbidden";
    reason_map_[404] = "404 Not Found";

}



int Mapper::GetContentType(string file_type)
{
    // 默认为0
    return content_type_map_[file_type];
}

string Mapper::GetURI(int code)
{
    // 默认为0
    string uri = uri_map_[code];
    return uri;
}

// string Mapper::GetURI(string request_uri)
// {
//     // 默认为0
//     string uri = uri_map_[request_uri];
//     if(uri == "")
//     {
//         uri = request_uri;
//     }
//     if(content_type < 10)
//     {
//         return TEMPLATES_DIR + uri;
//     }
//     else
//     {
//         return RESOURCES_DIR + uri;
//     }
// }

void Mapper::InitViewMap()
{
    view_map_["/"] = main_page;
    view_map_["/404/"] = error_404;
    view_map_["/403/"] = error_403;

}


View Mapper::GetView(string request_uri)
{
    View view = view_map_[request_uri];
    return view;
}

string & Mapper::GetReason(int code)
{
    return reason_map_[code];
}

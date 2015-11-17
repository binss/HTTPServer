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


Mapper::Mapper():logger_("Mapper", DEBUG, true)
{
    InitContentTypeMap();
    InitViewMap();
    InitReasonMap();
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
    content_type_map_["ico"] = 23;
}

void Mapper::InitReasonMap()
{
    // 0 - 9 chucked
    reason_map_[200] = "200 OK";
    reason_map_[304] = "304 Not Modified";
    reason_map_[403] = "403 Forbidden";
    reason_map_[404] = "404 Not Found";
    reason_map_[500] = "500 Internal Server Error";

}



int Mapper::GetContentType(string file_type)
{
    // 默认为0
    return content_type_map_[file_type];
}

void Mapper::InitViewMap()
{
    view_map_["/"] = main_page;
    view_map_["/404/"] = error_404;
    view_map_["/403/"] = error_403;

}


View Mapper::GetView(string target)
{
    View view = view_map_[target];
    if(NULL == view)
    {
        logger_<<"Can not find the view of the target["<<target<<"]"<<endl;
        return view_map_["/404/"];
    }
    return view;
}

string & Mapper::GetReason(int code)
{
    return reason_map_[code];
}

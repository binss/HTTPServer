/***********************************************************
* FileName:      View.cpp
* Author:        binss
* Create:        2015-11-10 21:47:55
* Description:   No Description
***********************************************************/

#include "View.h"
#include "Model.h"

Logger LOG("View", DEBUG, true);

void main_page(Request &request, Response &response)
{
    if(request.META.METHOD == "GET")
    {
        response.SetCookie("username", "binss", GetTime(60000));
        response.SetCookie("email", "i@binss.me", GetTime(60000));
        response.SetCode(200);
        response.SetFile("index.html");
    }
    else if(request.META.METHOD == "POST")
    {
        LOG<<DEBUG<<request.POST["name"]<<endl;
        response.SetCode(200);

        response.SetRawString("OK");
        // response.SetFile("index.html");

    }
    // LOG<<DEBUG<<"\nGET\n"<<request.GET<<endl;
    // LOG<<DEBUG<<"\nCOOKIE\n"<<request.COOKIE<<endl;
}

void upload_page(Request &request, Response &response)
{
    if(request.META.METHOD == "GET")
    {
        response.SetCode(200);
        response.SetFile("upload.html");
    }
    else if(request.META.METHOD == "POST")
    {
        LOG<<DEBUG<<request.POST["name"]<<endl;
        response.SetCode(200);
        response.SetRawString("OK");
    }
}


void error_404(Request &request, Response &response)
{
    response.SetCode(404);
    response.SetFile("error/404.html");
}

void error_403(Request &request, Response &response)
{
    response.SetCode(403);
    response.SetFile("error/403.html");
}


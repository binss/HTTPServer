/***********************************************************
* FileName:      View.cpp
* Author:        binss
* Create:        2015-11-10 21:47:55
* Description:   No Description
***********************************************************/

#include "View.h"


void main_page(Request &request, Response &response)
{
    response.SetCookie("username", "binss", GetTime(0));
    response.SetCookie("email", "i@binss.me", GetTime(0));
    response.SetFile("index.html");
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

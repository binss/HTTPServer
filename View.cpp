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

/***********************************************************
* FileName:      View.h
* Author:        binss
* Create:        2015-11-10 17:36:39
* Description:   No Description
***********************************************************/


#ifndef  __VIEW_H__
#define  __VIEW_H__


#include "Request.h"
#include "Response.h"
#include "Logger.h"

typedef void (* View)(Request &, Response &);

void main_page(Request &request, Response &response);
void error_403(Request &request, Response &response);
void error_404(Request &request, Response &response);


#endif

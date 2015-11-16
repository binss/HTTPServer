/***********************************************************
* FileName:      Connection.h
* Author:        binss
* Create:        2015-11-10 17:47:10
* Description:   No Description
***********************************************************/

#ifndef  __CONNECTION_H__
#define  __CONNECTION_H__


#include "Request.h"
#include "Response.h"
#include "GlobalUtil.h"


class Connection
{
public:
    Connection(int sockfd, char * host);
    ~Connection();
    int PreSend();
    int PostRecv();
    int End();
    int Reset();
    int Close();

public:
    int recv_length;
    int send_length;
    char * pBuffer;

private:
    int sockfd_;
    Request request_;
    Response response_;
    string host_;
    int time_;
    Logger logger_;
};

#endif

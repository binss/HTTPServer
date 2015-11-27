/***********************************************************
* FileName:      Connection.h
* Author:        binss
* Create:        2015-11-10 17:47:10
* Description:   No Description
***********************************************************/

#ifndef  __CONNECTION_H__
#define  __CONNECTION_H__

#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#include "Request.h"
#include "Response.h"
#include "GlobalUtil.h"


class Connection
{
public:
    Connection(int sockfd, char * host);
    ~Connection();
    int Send();
    int Recv();
    int End();
    int Reset();
    int Close();

private:
    int AddRecvLength(int length);
    int SetTimer();
    int ResetTimer();
    static void TimeOut(union sigval sig);

public:
    bool pending;
    bool erroring;

private:
    int sockfd_;
    Request request_;
    Response response_;
    string host_;
    int recv_length_;
    int send_length_;
    Logger logger_;
    timer_t timer_;
};

#endif

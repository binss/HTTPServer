/***********************************************************
* FileName:      Connection.cpp
* Author:        binss
* Create:        2015-11-10 17:47:40
* Description:   No Description
***********************************************************/

#include "Connection.h"
#include "Mapper.h"

Connection::Connection(int sockfd, char * host):sockfd_(sockfd), host_(host), logger_("Connection", DEBUG, true)
{
    recv_length = 0;
    send_length = 0;
    time_ = time(0);
    pBuffer = request_.GetBuffer();
}

Connection::~Connection()
{
    pBuffer = NULL;
}

int Connection::PostRecv()
{
    return request_.Parse(recv_length);
}

int Connection::PreSend()
{
    int ret = response_.Init(request_);
    if( 0 == ret )
    {
        if( 0 == response_.GetType())
        {
            View view = Mapper::GetInstance()->GetView(response_.GetTarget());
            if( NULL == view)
            {
                logger_<<CRITICAL<<"The view is NULL"<<endl;
            }
            else
            {
                view(request_, response_);
            }
        }
        else
        {
            response_.SetCode(200);
        }

        ret = response_.Build();
        if( 0 == ret )
        {
            pBuffer = response_.GetBuffer();
            return response_.GetBufferLength();
        }
    }
    return ret;
}

int Connection::End()
{
    logger_<<INFO<<host_<<"  \""<<request_.METHOD<<" "<<request_.RAW_URI<<" "<<request_.PROTOCOL<<"\" "<<response_.GetCode()<<" "<<response_.GetContentLength()<<endl;
    if(!response_.GetKeepAlive())
    {
        Close();
        return -1;
    }
    return 0;
}

int Connection::Reset()
{
    time_ = time(0);
    recv_length = 0;
    send_length = 0;
    request_.Reset();
    response_.Reset();
    pBuffer = request_.GetBuffer();
    return 0;
}

int Connection::Close()
{
    // 关闭socket
    logger_<<DEBUG<<"Connection["<<sockfd_<<"] closed"<<endl;
    close(sockfd_);
    return 0;
}


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
    recv_length_ = 0;
    send_length_ = 0;
    pending = false;
    erroring = false;
    SetTimer();
}

Connection::~Connection()
{
}

int Connection::Recv()
{
    int ret;
    while(true)
    {
        Byte * pBuffer = request_.GetBuffer();
        int length = recv(sockfd_, pBuffer + recv_length_, request_.GetBufferSize() - recv_length_, 0);
        if(length > 0)
        {
            ret = AddRecvLength(length);
            if( ret != E_Suc )
            {
                logger_<<ERROR<<"AddRecvLength error, ret: "<<ret<<endl;
                return E_Server_Close;
            }
        }
        else if(length < 0)
        {
            // 数据读取完毕
            break;
            // -1 ERRNO 11 代表socket 未可读
        }
        else if(length == 0)
        {
            // 如果收到0，代表客户端主动断开
            logger_<<INFO<<"Client close the connection"<<endl;
            return E_Client_Close;
        }
    }
    logger_<<VERBOSE<<"Recv Complete. Length: "<<recv_length_<<" fd: "<<sockfd_<<endl;

    if(pending)
    {
        ret = request_.Append(recv_length_);
    }
    else
    {
        ret = request_.Parse(recv_length_);
    }
    return ret;
}

int Connection::Send()
{
    int ret = response_.Init(&request_.HEADER, &request_.COOKIE, &request_.META);
    if( ret == E_Suc )
    {
        if( response_.TYPE == 0 )
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
        if( ret != E_Suc )
        {
            return ret;
        }
    }

    Byte * pBuffer = response_.GetBuffer();
    int length = response_.GetBufferLength();
    if( pBuffer == NULL || length < 0 )
    {
        // error, close the socket
        return E_Buffer_Error;
    }

    while(true)
    {
        //检查有无已读取还未写入的
        int remain_length = length - send_length_;
        if (remain_length > 0)
        {
            int lenght = send(sockfd_, pBuffer + send_length_, remain_length, 0);
            send_length_ += lenght;
            if(lenght != remain_length)
            {
                // 缓冲区已满，返回
                return E_Buffer_Error;
            }
        }
        else
        {
            // 已经写完
            break;
        }
    }
    logger_<<VERBOSE<<"Send Complete. Length: "<<send_length_<<" fd: "<<sockfd_<<endl;

    return E_Suc;
}

int Connection::End()
{
    Meta & meta = request_.META;
    logger_<<INFO<<host_<<"  \""<<meta.METHOD<<" "<<meta.RAW_URI<<" "<<meta.PROTOCOL<<"\" "<<response_.CODE<<" "<<response_.GetContentLength()<<endl;
    pending = false;
    if(!meta.KEEP_ALIVE)
    {
        Close();
        return E_Server_Close;
    }
    return E_Suc;
}

int Connection::Reset()
{
    ResetTimer();
    recv_length_ = 0;
    send_length_ = 0;
    request_.Reset();
    response_.Reset();
    return E_Suc;
}

int Connection::Close()
{
    // 关闭socket
    logger_<<DEBUG<<"Connection["<<sockfd_<<"] closed"<<endl;
    close(sockfd_);
    return E_Suc;
}

int Connection::AddRecvLength(int length)
{
    int recv_buffer_size_ = request_.GetBufferSize();
    if(recv_length_ + length >= recv_buffer_size_)
    {
        recv_buffer_size_ = recv_buffer_size_ * 2;
        int ret = request_.EnlargeBuffer(recv_buffer_size_);
        if(ret)
        {
            return ret;
        }
    }
    recv_length_ += length;
    return E_Suc;
}

void Connection::TimeOut(union sigval sig)
{
    Connection *obj = (Connection *)sig.sival_ptr;
    obj->logger_<<DEBUG<<"Connection Timeout"<<endl;
    obj->Close();
    timer_delete(obj->timer_);
}

int Connection::ResetTimer()
{
    itimerspec itimer;
    itimer.it_value.tv_sec = Connection_Alive_Time;
    itimer.it_value.tv_nsec = 0;

    if(timer_settime(timer_, 0, &itimer, NULL) < 0 )
    {
        logger_<<ERROR<<"Set timer failed. errno:"<<errno<<endl;
        return E_Timer_Error;
    }
    return E_Suc;
}

int Connection::SetTimer()
{
    struct sigevent sigev;
    memset(&sigev, 0, sizeof (struct sigevent));
    // 以sockfd作为定时器id
    sigev.sigev_value.sival_ptr = this;

    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_notify_attributes = NULL;
    sigev.sigev_notify_function = TimeOut;

    if( timer_create(CLOCK_REALTIME, &sigev, &timer_) < 0 )
    {
        logger_<<ERROR<<"Create Timer failed. errno:"<<errno<<endl;
        return E_Timer_Error;
    }

    ResetTimer();
    return E_Suc;
}

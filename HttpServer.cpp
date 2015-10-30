#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>  /* basic socket definitions */
#include <sys/time.h>    /* timeval{} for select() */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "Request.h"
#include "Response.h"

using namespace std;

#define SERV_PORT 8888
#define LISTENQ 1024    /* 2nd argument to listen() */
#define EPOLL_SIZE 1024
#define EPOLL_TIMEOUT 500

int listenfd, epfd;

class Connection
{
public:
    Connection(int sockfd=0):fd(sockfd)
    {
    }
    int BuildRequest(char *buf, int len)
    {
        return request_.Parse(buf, len);
    }

    int BuildResponse()
    {
        int ret = response_.Init(request_.GetHeader());
        if( 0 == ret)
        {
            return response_.Build();
        }
        return ret;
    }
    string & GetResponse()
    {
        return response_.GetStr();
    }
public:
    int fd;
    Request request_;
    Response response_;
};

unordered_map<int, Connection> connections;

int HandleHttpRequest(int epfd, int sockfd)
{
    int ret;
    int len;
    errno = 0;
    while(true)
    {
        char buffer[1024*8] = {0};
        len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if(len > 0)
        {
            printf("[HandleHttpRequest]: len: %d errno: %d, fd: %d\n", len, errno, sockfd);
        }
        else if(len < 0)
        {
            // 数据读取完毕
            break;
            // -1 ERRNO 11 代表socket 未可读
        }
        else if(len == 0)
        {
            // 如果收到0，代表客户端主动断开
            close(sockfd);
            return 0;
        }
        Connection connection(sockfd);
        connection.BuildRequest(buffer, strlen(buffer));
        connections[sockfd] = connection;
    }

    struct epoll_event event;
    event.data.fd = sockfd;
    event.events = EPOLLOUT | EPOLLERR | EPOLLET;
    ret = epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &event);
    if(ret != 0)
    {
        printf("[HandleHttpRequest]epoll_ctl error\n");
    }
    return 0;
}

int HandleHttpResponse(int epfd, int sockfd)
{
    errno = 0;

    Connection & connection = connections[sockfd];
    connection.BuildResponse();
    string & response_str = connection.GetResponse();

    int len = send(sockfd, response_str.c_str(), response_str.length(), 0);
    printf("[HandleHttpResponse]: len: %d errno: %d, fd: %d\n", len, errno, sockfd);

    // struct epoll_event event;
    // event.data.fd = sockfd;
    // event.events = EPOLLIN | EPOLLERR;
    // int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &event);
    // if(ret != 0)
    // {
    //     printf("[HandleHttpResponse]epoll_ctl error, ret: %d, errno: %d\n", ret, errno);
    // }
    close(sockfd);
    return 0;
}

int SetNonblocking(int fd)
{
    int flags;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIONBIO, &flags);
#endif
}


int HandleNewRequest(int listenfd)
{
    while(true)
    {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);
        if(connfd == -1)
        {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
            {
                // 已处理完此次ET的所有请求
                break;
            }
            else
            {
                printf("accept error, errno: %d\n", errno);
                break;
                }
            break;
        }
        SetNonblocking(connfd);

        int on = 1;
        // 停用Nagle算法
        setsockopt(connfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

        char dest[30];
        inet_ntop(AF_INET, &cliaddr.sin_addr, dest, 30);
        printf("[new]socket: %s, port %d, fd: %d\n", dest, ntohs(cliaddr.sin_port), connfd);

        // 接受连接
        struct epoll_event event;
        event.data.fd = connfd;
        event.events = EPOLLIN | EPOLLET;
        // 把新连接的描述符也加入epoll
        int iRet = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
        if ( iRet == -1 )
        {
            printf("epoll_ctl error\n");
            break;
        }
    }
    return 0;

}

void sighandler ( int sig )
{
    close(listenfd);
    printf("closed\n");
    exit(0);
}




int main()
{
    signal ( SIGABRT, &sighandler );
    signal ( SIGTERM, &sighandler );
    signal ( SIGINT, &sighandler );


    int event_count;
    struct sockaddr_in servaddr;
    struct epoll_event event, events[EPOLL_SIZE];
    // 初始化监听socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    SetNonblocking(listenfd);

    // 重用端口
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);
    // 绑定监听描述符
    int ret = bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));
    if(ret == -1)
    {
        printf("[server]bind error ret: %d\n", errno);

    }
    // 开始监听
    if(listen(listenfd, LISTENQ))
    {
        printf("listen error\n");
        return 0;
    }

    printf("[server]listening: %d:%d\n", INADDR_ANY, SERV_PORT);

    // 创建epoll
    epfd = epoll_create(EPOLL_SIZE);

    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    // 把监听描述符绑定到epoll上
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event))
    {
        cout<<"epoll add fd error"<<endl;
        return 0;
    }
    for( ; ; )
    {
        // 如有描述符就绪
        event_count = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_TIMEOUT);

        for(int i = 0; i < event_count; i++)
        {
            // 出错
            if ( (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) )
            {
                printf("epoll error\n");
                close(events[i].data.fd);
                continue;
            }

            // 如果就绪的是监听描述符
            if(events[i].data.fd == listenfd)
            {
                HandleNewRequest(listenfd);
                // clients_list.push_back(connfd); // 添加新的客户端到list
            }
            else
            {
                printf("[wake]socket: fd: %d event: %d\n", events[i].data.fd, events[i].events);
                // 如果是和client连接的描述符
                if(events[i].events & EPOLLIN)
                {
                    HandleHttpRequest(epfd, events[i].data.fd);
                }
                else if(events[i].events & EPOLLOUT)
                {
                    HandleHttpResponse(epfd, events[i].data.fd);
                }
                else if(events[i].events & EPOLLERR)
                {
                    printf("error\n");
                }
            }
        }
    }
    return 0;
}

#include <iostream>
#include <sys/socket.h>  /* basic socket definitions */
#include <sys/time.h>    /* timeval{} for select() */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include "RequestHandler.h"

using namespace std;

#define SERV_PORT 8888
#define LISTENQ 1024    /* 2nd argument to listen() */
#define EPOLL_SIZE 1024
#define EPOLL_TIMEOUT 500



void HandleHttpRequest(int sockfd)
{
    int ret;
    char buffer[1024*8] = {0};
    ret = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if(ret > 0)
    {
        RequestHandler request_handler;

        request_handler.ParseRequest(buffer, strlen(buffer));
        unordered_map<string, string> header = request_handler.GetHttpHeader();
        // cout<< header["Cookie"]<<endl;
        FILE * fp = fdopen(sockfd, "w");
        if( fp == NULL )
        {
            cout <<"bad fp"<<endl;
        }
        else
        {
            // path和长度不一致时会变成下载?
            fwrite(buffer, strlen(buffer), 1, fp);
        }
        fclose(fp);
    }
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

int Epoll()
{
    int listenfd, connfd, epfd, event_count;
    struct sockaddr_in cliaddr, servaddr;
    struct epoll_event event, events[EPOLL_SIZE];
    socklen_t clilen;
    // 初始化监听socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    SetNonblocking(listenfd);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);
    // 绑定监听描述符
    bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));

    // 开始监听
    if(listen(listenfd, LISTENQ))
    {
        cout<<"listen error"<<endl;
        return 0;
    }
    cout<<"listening: "<<INADDR_ANY<<":"<<SERV_PORT<<endl;

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
            // 如果就绪的是监听描述符
            if(events[i].data.fd == listenfd)
            {
                clilen = sizeof(cliaddr);
                connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);
                SetNonblocking(connfd);

                char dest[30];
                inet_ntop(AF_INET, &cliaddr.sin_addr, dest, 30);
                printf("new client: %s, port %d\n", dest, ntohs(cliaddr.sin_port));
                // 接受连接
                event.data.fd = connfd;
                // 把新连接的描述符也加入epoll
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                // clients_list.push_back(connfd); // 添加新的客户端到list
                cout<<"new connfd:"<<connfd<<endl;
                HandleHttpRequest(connfd);

            }
            else
            {
                // 如果是和client连接的描述符
                HandleHttpRequest(events[i].data.fd);
            }
        }
    }
}

int Select()
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    fd_set rset, allset;
    socklen_t clilen;
    struct sockaddr_in  cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);

    bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr));

    if(listen(listenfd, LISTENQ))
    {
        cout<<"error"<<endl;
        return 0;
    }
    cout<<"listening: "<<INADDR_ANY<<":"<<SERV_PORT<<endl;

    maxfd = listenfd;           /* initialize */
    maxi = -1;                  /* index into client[] array */
    // 初始化客户数组的值为-1代表空
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;         /* -1 indicates available entry */
    FD_ZERO(&allset);
    // 把监听描述符加入读取集
    FD_SET(listenfd, &allset);


    for ( ; ; )
    {
        rset = allset;      /* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        // 如果有套接字可读
        // 如果是监听套接字可读，代表有新客户连接
        if (FD_ISSET(listenfd, &rset))
        {    /* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);
            char dest[30];
            inet_ntop(AF_INET, &cliaddr.sin_addr, dest, 30);
            printf("new client: %s, port %d\n", dest, ntohs(cliaddr.sin_port));
            // 寻找client数组第一个空位并设置为该客户的套接字描述符
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd; /* save descriptor */
                    break;
                }
            }
            // 如果client数组已满，报错
            if (i == FD_SETSIZE)
                cout<<"too many clients"<<endl;
            // 把该客户的套接字加入读取集
            FD_SET(connfd, &allset);    /* add new descriptor to set */
            // 设置检查的套接字数目（+1）
            if (connfd > maxfd)
                maxfd = connfd;         /* for select */
            // 设置client中最大的描述符序号
            if (i > maxi)
                maxi = i;               /* max index in client[] array */
            // 如果没有其他描述符就绪，返回（等待下一次select返回
            if (--nready <= 0)
                continue;               /* no more readable descriptors */
        }
        // 遍历client数组（前maxi个）
        for (i = 0; i <= maxi; i++)
        {   /* check all clients for data */
            // 空（-1），继续
            if ( (sockfd = client[i]) < 0)
                continue;
            // 如果该描述符就绪，读取并回显写入
            if (FD_ISSET(sockfd, &rset))
            {
                HandleHttpRequest(sockfd);

                if (--nready <= 0)
                    break;              /* no more readable descriptors */
            }
        }
    }
}

int main()
{
    // Select();
    Epoll();
    return 0;
}

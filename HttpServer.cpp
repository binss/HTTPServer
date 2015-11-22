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

#include "Connection.h"

int listenfd, epfd;


unordered_map<int, Connection *> connections;
Logger logger("Server", DEBUG, true);

int HandleHttpRequest(int epfd, int sockfd)
{
    int ret;
    errno = 0;
    unordered_map<int, Connection *>::iterator search = connections.find(sockfd);
    Connection *pConnection;
    if(search != connections.end())
    {
        pConnection = search->second;
        if(!pConnection->pending)
        {
            pConnection->Reset();
        }
    }
    else
    {
        // 没找到
        logger<<ERROR<<"Can not find sockfd["<<sockfd<<"]"<<endl;
        close(sockfd);
        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
        if(ret != 0)
        {
            printf("epoll_ctl error, ret: %d, errno: %d\n", ret, errno);
        }
        return -1;
    }

    ret = pConnection->Recv();
    if(ret == 0)
    {
        struct epoll_event event;
        event.data.fd = sockfd;
        event.events = EPOLLOUT | EPOLLERR | EPOLLET;
        ret = epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &event);
        if(ret != 0)
        {
            logger<<ERROR<<"epoll_ctl error, ret: "<<ret<<" errno: "<<errno<<endl;
        }
    }
    else if(ret != E_Request_Not_Complete && ret != E_Client_Close)
    {
        logger<<ERROR<<"Recv error. ret "<<ret<<endl;
        pConnection->Close();
        return -1;
    }

    return 0;
}

int HandleHttpResponse(int epfd, int sockfd)
{
    // errno = 0;
    Connection *pConnection = connections[sockfd];
    int ret = pConnection->Send();
    if(ret)
    {
        logger<<ERROR<<"Send error. ret "<<ret<<endl;
    }
    ret = pConnection->End();
    if(ret)
    {
        connections.erase(sockfd);
        delete pConnection;
    }
    else
    {
        struct epoll_event event;
        event.data.fd = sockfd;
        event.events = EPOLLIN | EPOLLERR | EPOLLET;
        int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &event);
        if(ret != 0)
        {
            logger<<ERROR<<"epoll_ctl error, ret: "<<ret<<" errno: "<<errno<<endl;
        }
    }

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
        int sockfd = accept(listenfd, (sockaddr *) &cliaddr, &clilen);
        if(sockfd == -1)
        {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
            {
                // 已处理完此次ET的所有请求
                break;
            }
            else
            {
                logger<<ERROR<<"HandleNewRequest: accept error, errno: "<<errno<<endl;
                break;
                }
            break;
        }
        SetNonblocking(sockfd);

        int on = 1;
        // 停用Nagle算法
        setsockopt(sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

        char host[30];
        inet_ntop(AF_INET, &cliaddr.sin_addr, host, 30);

        logger<<DEBUG<<"HandleNewRequest: new socket: "<<host<<", port: "<<ntohs(cliaddr.sin_port)<<" fd: "<<sockfd<<endl;

        // 为socket建立connection
        connections[sockfd] = new Connection(sockfd, host);

        // 把新连接的描述符也加入epoll
        struct epoll_event event;
        event.data.fd = sockfd;
        event.events = EPOLLIN | EPOLLET;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
        if ( ret != 0 )
        {
            // == -1
            logger<<ERROR<<"HandleNewRequest: epoll_ctl error, ret: "<<ret<<endl;
            break;
        }
    }
    return 0;

}

void sighandler(int sig)
{
    close(listenfd);
    for(unordered_map<int, Connection *>::iterator iter = connections.begin(); iter != connections.end(); ++iter)
    {
        close(iter->first);
    }

    logger<<INFO<<"Http Server closed"<<endl;
    exit(0);
}


int main()
{
    signal ( SIGABRT, &sighandler );
    signal ( SIGTERM, &sighandler );
    signal ( SIGINT, &sighandler );

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
        logger<<CRITICAL<<"Bind error, errno:"<<errno<<endl;
    }
    // 开始监听
    if(listen(listenfd, LISTENQ))
    {
        logger<<CRITICAL<<"Listen error, errno:"<<errno<<endl;
        return -1;
    }

    logger<<INFO<<"Listening:"<<INADDR_ANY<<":"<<SERV_PORT<<endl;

    // 创建epoll
    epfd = epoll_create(EPOLL_SIZE);

    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    // 把监听描述符绑定到epoll上
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event))
    {
        logger<<CRITICAL<<"Epoll add fd error, errno:"<<errno<<endl;
        return -2;
    }
    for( ; ; )
    {
        // 如有描述符就绪
        int event_count = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_TIMEOUT);

        for(int i = 0; i < event_count; i++)
        {
            // 出错
            if ( (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) )
            {
                logger<<ERROR<<"Epoll error"<<endl;
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
                logger<<VERBOSE<<"Socket wake: fd:"<<events[i].data.fd<<" event: "<<events[i].events<<endl;
                // 如果是和client连接的描述符
                if(events[i].events & EPOLLIN)
                {
                    HandleHttpRequest(epfd, events[i].data.fd);
                }
                else if(events[i].events & EPOLLOUT)
                {
                    HandleHttpResponse(epfd, events[i].data.fd);
                }
            }
        }
    }
    return 0;
}


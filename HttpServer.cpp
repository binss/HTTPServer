#include <iostream>
#include <sys/socket.h>  /* basic socket definitions */
#include <sys/time.h>    /* timeval{} for select() */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include "RequestHandler.h"

using namespace std;

#define SERV_PORT 8888
#define LISTENQ 1024    /* 2nd argument to listen() */


void DealHttpRequest(int sockfd)
{
    int ret;
    char buffer[1024*8] = {0};
    ret = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if(ret > 0)
    {
        RequestHandler request_handler;

        request_handler.ParseRequest(buffer, strlen(buffer));
        // char path[128] = {0};

        // char *line = strtok(buffer, "\n");
        // while(line != NULL)
        // {
        //     if(strncmp(line, "GET ", 3) == 0)
        //     {
        //         printf("SERVER: GET request\n");
        //         char *pos = strstr(line + 4, " ");
        //         int len = pos - line - 4;
        //         strncpy(path, line + 4, len);
        //     }
        //     printf("log: %s\n", line);
        //     line = strtok(NULL, "\n");
        // }
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

int main()
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
                if (client[i] < 0) {
                    client[i] = connfd; /* save descriptor */
                    break;
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
                DealHttpRequest(sockfd);

                if (--nready <= 0)
                    break;              /* no more readable descriptors */
            }
        }
    }
}

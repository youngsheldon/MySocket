/* 
* @Author: anchen
* @Date:   2016-11-02 15:13:51
* @Last Modified by:   anchen
* @Last Modified time: 2016-11-02 22:54:03
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 8888
#define MAX_LINE 2048
#define LISTENQ 1024
using namespace std;

class Server
{
public:
    Server(int port);
    void selectModeSet(void);
    static void *clientLinkHandle(void *obj_temp);
    static void *clientRequestHandle(void *obj_temp);
    ~Server(){};
private:
    int listenfd,connfd;
    int port;
    fd_set rset,allset;
    int maxfd;
    int index_max,index;
    int client[FD_SETSIZE];
    pthread_t client_link_tid , client_request_tid;
};

Server::Server(int port)
{
    struct sockaddr_in servaddr;
    this->port = port;
    this->listenfd = socket(AF_INET , SOCK_STREAM , 0);
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(this->port);
    unsigned int value = 0x1;
    setsockopt(this->listenfd,SOL_SOCKET,SO_REUSEADDR,(void *)&value,sizeof(value));
    bind(this->listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr));
    listen(this->listenfd , LISTENQ);
}

void Server::selectModeSet(void)
{
    FD_ZERO(&this->allset);
    FD_SET(this->listenfd , &this->allset);
    this->maxfd = this->listenfd;
    this->index_max = -1;
    for (int i = 0; i < FD_SETSIZE; ++i)
    {
        this->client[i] = -1;
    }
    if(pthread_create(&this->client_link_tid , NULL , this->clientLinkHandle, this) == -1)
    {
        perror("pthread create error.\n");
        exit(1);
    }
    if(pthread_create(&this->client_request_tid , NULL , this->clientRequestHandle, this) == -1)
    {
        perror("pthread create error.\n");
        exit(1);
    }
}

void *Server::clientRequestHandle(void *obj_temp)
{
    Server *obj = (Server *)obj_temp;
    int n_ready;
    while(1)
    {
        obj->rset = obj->allset;
        n_ready = select(obj->maxfd+1 , &obj->rset , NULL , NULL , NULL);
        for(int i = 0; i <=obj->index_max; i++)
        {
            int sockfd = obj->client[i];
            if(FD_ISSET(sockfd , &obj->rset))
            {
                printf("\nreading the socket~~~ \n");
                char buf[MAX_LINE];
                bzero(buf , MAX_LINE);
                ssize_t n;
                if((n = read(sockfd , buf , MAX_LINE)) <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd , &obj->allset);
                    obj->client[i] = -1;
                }
                else
                {
                    ssize_t ret;
                    printf("clint[%d] send message: %s\n", i , buf);
                    if((ret = write(sockfd , buf , n)) != n)    
                    {
                        printf("error writing to the sockfd!\n");
                        break;
                    }
                }
                if(--n_ready <= 0)
                    break;
            }
        }
    }
}

void *Server::clientLinkHandle(void *obj_temp)
{
    Server *obj = (Server *)obj_temp;
    while(1)
    {
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        if((obj->connfd = accept(obj->listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
        {
            perror("accept error.\n");
            exit(1);
        }   
        printf("accpet a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr) , cliaddr.sin_port);
        for(obj->index = 0 ; obj->index < FD_SETSIZE ; ++obj->index)
        {
            if(obj->client[obj->index] < 0)
            {
                obj->client[obj->index] = obj->connfd;
                break;
            }
        }
        FD_SET(obj->connfd , &obj->allset);
        if(obj->connfd > obj->maxfd)
            obj->maxfd = obj->connfd;
        if(obj->index > obj->index_max)
            obj->index_max = obj->index;
        printf("index_max = %d\n", obj->index_max);
    }
}

int main()
{
    Server server(PORT);
    server.selectModeSet();
    while(1)
    {

    }
    return 0;
} 

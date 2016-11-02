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


#define PORT 8888
#define MAX_LINE 2048
#define LISTENQ 1024


int main(int argc , char **argv)
{
    int listenfd,connfd,sockfd;
    struct sockaddr_in servaddr , cliaddr;
    socklen_t clilen;
    fd_set rset,allset;
    int n_ready,maxfd,index,index_max;
    int client[FD_SETSIZE];
    char buf[MAX_LINE];

    /*(1) 得到监听描述符*/
    listenfd = socket(AF_INET , SOCK_STREAM , 0);

    /*(2) 绑定套接字*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    unsigned int value = 0x1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void *)&value,sizeof(value));

    bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr));

    /*(3) 监听*/
    listen(listenfd , LISTENQ);

    FD_ZERO(&allset);
    FD_SET(listenfd , &allset);
    maxfd = listenfd;
    index_max = -1;
    
    for (int i = 0; i < FD_SETSIZE; ++i)
    {
        client[i] = -1;
    }

    while(1)
    {
        rset = allset;
        n_ready = select(maxfd+1 , &rset , NULL , NULL , NULL);
       
        if(FD_ISSET(listenfd , &rset))
        {
            clilen = sizeof(cliaddr);
            printf("\naccpet connection~\n");

            if((connfd = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
            {
                perror("accept error.\n");
                exit(1);
            }   

            printf("accpet a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr) , cliaddr.sin_port);

            for(index = 0 ; index < FD_SETSIZE ; ++index)
            {
                if(client[index] < 0)
                {
                    client[index] = connfd;
                    break;
                }
            }
            FD_SET(connfd , &allset);
            if(connfd > maxfd)
                maxfd = connfd;

            if(index > index_max)
                index_max = index;

            printf("index_max = %d\n", index_max);

            if(--n_ready < 0)
                continue;
        }

        for(int i = 0; i <=index_max; i++)
        {
            sockfd = client[i];
            if(FD_ISSET(sockfd , &rset))
            {
                printf("\nreading the socket~~~ \n");
                bzero(buf , MAX_LINE);
                ssize_t n;
                if((n = read(sockfd , buf , MAX_LINE)) <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd , &allset);
                    client[i] = -1;
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

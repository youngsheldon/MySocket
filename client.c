/* 
* @Author: anchen
* @Date:   2016-11-01 15:46:18
* @Last Modified by:   anchen
* @Last Modified time: 2016-11-01 17:24:12
*/

#include "config.h"

ssize_t readline(int fd, char *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) 
    {
        if ( (rc = read(fd, &c,1)) == 1) 
        {
            *ptr++ = c;
            if (c == '\n')
                break;  /* newline is stored, like fgets() */
        } 
        else if (rc == 0) 
        {
            *ptr = 0;
            return(n - 1);  /* EOF, n - 1 bytes were read */
        } 
        else
            return(-1);     /* error, errno set by read() */
    }

    *ptr = 0;   /* null terminate like fgets() */
    return(n);
}

int main(int argc , char ** argv)
{
    int sockfd;

    if(argc != 2)
    {
        perror("usage:tcpcli <IPaddress>");
        exit(1);
    }

    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET , argv[1] , &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }

    if(connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }

    char sendline[MAX_LINE] , recvline[MAX_LINE];
    while(fgets(sendline , MAX_LINE , stdin) != NULL)   
    {
        write(sockfd , sendline , strlen(sendline));

        if(readline(sockfd , recvline , MAX_LINE) == 0)
        {
            perror("server terminated prematurely");
            exit(1);
        }

        if(fputs(recvline , stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        }
    }

    close(sockfd);

    return 0;
}
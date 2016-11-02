/* 
* @Author: anchen
* @Date:   2016-11-01 15:46:05
* @Last Modified by:   anchen
* @Last Modified time: 2016-11-01 18:11:17
*/

#include "config.h"

int main() 
{
    int listen_fd,client_fd;
    struct sockaddr_in servaddr , cliaddr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT);
    
    if (bind(listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind error");
        exit(1);
    }

    if(listen(listen_fd, BACKLOG) < 0)
    {
        perror("listen error");
        exit(1);
    }

    while(1)
    {
        socklen_t cliaddr_len = sizeof(cliaddr);
        if((client_fd = accept(listen_fd, (struct sockaddr*)&cliaddr, &cliaddr_len)) < 0)
        {
            perror("accept error");
            exit(1);
        }

        if((fork()) == 0)
        {
            char buf[1024];
            ssize_t n;
            while ((n = read(client_fd, buf, sizeof(buf))) > 0)
            {
                write(client_fd , buf , n);
                printf("%s\n", buf);
            }
            exit(0);
        }
        close(client_fd);
    }
    close(listen_fd);

    return 0;
}
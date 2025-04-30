#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>


int main()
{
    int                 sfd;
    int                 afd;
    int                 addr_size;
    struct sockaddr_in  *sa;
    
    char            buf[256];
    
    sfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sfd < 0)
        return (-1);
    
    printf("socket: %d\n", sfd);
    sa = calloc(sizeof(struct sockaddr_in), 1);
    
    addr_size = sizeof(sa);
    bind(sfd, sa, sizeof(sa));
    while (1)
    {
        if (listen(sfd, 256))
        {
            printf("listen finish");
            break;
        }
    }
                
    afd = accept(sfd, sa,(socklen_t *) &addr_size);
    recv(afd, buf, 256, MSG_WAITALL);
    printf("%s\n", buf);
    close(afd);
    free(sa);
    close(sfd);
}
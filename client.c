#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    int  sfd;
    struct sockaddr *sa;
    char msg[256];
    sfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sfd < 0)
        return (1);
    sa = calloc(sizeof(struct sockaddr), 1);
    int cn = connect(sfd, sa, sizeof(sa));
    printf("%d\n", cn);
    while (1)
    {
        printf("Waiting to send: ");
        scanf("%s", msg);
        send(sfd, msg, sizeof(msg), MSG_BATCH);
    }
    
}
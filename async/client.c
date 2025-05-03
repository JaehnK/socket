#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    int                 socket_fd;
    struct sockaddr_in *socket_addr;
    char                buf[256];
    int                 epfd, nfds;
    struct epoll_event  ev, events[10];

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        write(1, "socket fd\n", 11);
        return (-1);
    }
    
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags| O_NONBLOCK);

    socket_addr = calloc(sizeof(struct sockaddr_in), 1);
    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(8080);
    
    if (inet_pton(AF_INET, "127.0.0.1", &socket_addr->sin_addr) <= 0)
    {
        write(1, "inet_pton\n", 11);
        return (-1);
    }

    int result = connect(socket_fd, (struct sockaddr *)socket_addr, sizeof(struct sockaddr_in));
    if (result < 0 && errno != EINPROGRESS)
    {
        perror("connect");
        return (-1);
    }

    epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll");
        return (-1);
    }
    epoll_ctl(epfd, )

    printf("Waiting to send: ");
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0'; 
    send(socket_fd, buf, strlen(buf), 0);

    free(socket_addr);
    close(socket_fd);
}
//https://claude.ai/chat/8852b4ac-f76f-4aea-9d16-f39096bc24ea
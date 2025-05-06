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

#define MAX_EVENTS 10

int     open_socket()
{
    int socket_fd;

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        return (-1);
    }
    return (socket_fd);
}

int     init_async(int socket_fd, struct sockaddr_in *socket_addr)
{
    int flag;
    int result;

    flag = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flag | O_NONBLOCK);

    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &socket_addr->sin_addr) <= 0)
    {
        perror("inet_pton");
        return (-1);
    }
    
    result = connect(socket_fd, (struct sockaddr *)socket_addr, sizeof(struct sockaddr_in));
    if (result < 0 && errno != EINPROGRESS)
    {
        perror("connect");
        return (-1);
    }

    return (0);
}

int     init_epoll(int socket_fd, struct epoll_event *ev)
{
    int epfd;

    epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create");
        return (-1);
    }
    
    ev->events = EPOLLOUT | EPOLLIN;
    ev->data.fd = socket_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, ev) == -1)
    {
        perror("epoll_ctl: socket_fd");
        return (-1);
    }

    ev->events = EPOLLIN;
    ev->data.fd = STDIN_FILENO;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, ev) == -1)
    {
        perror("epoll_ctl: stdin");
        return (-1);
    }

    return (epfd);
}

void    free_close_exit(int socket_fd, int epfd, int flag)
{
    close(socket_fd);
    close(epfd);
    exit(flag);
}

int main(void)
{
    int                 socket_fd;
    struct sockaddr_in socket_addr;
    char                buf[256];
    int                 epfd, nfds;
    struct epoll_event  ev, events[10];
    
    int connected, sent, errflag;
    socklen_t   size;

    socket_fd = open_socket();
    if (socket_fd == -1)
        return (-1);
    
    if (init_async(socket_fd, &socket_addr) == -1)
        return (-1);

    if ((epfd = init_epoll(socket_fd, &ev)) == -1)
        return (-1);

    connected = 0;
    sent = 0;
    size = sizeof(errflag);
    while (1)
    {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            printf("epfd: %d\n", epfd);
            perror("epoll_wait");
            break ;
        }
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == socket_fd && \
                (events[i].events & EPOLLOUT))
            {
                if (connected == 0)
                {
                    if (getsockopt(socket_fd, SOL_SOCKET, \
                            SO_ERROR, &errflag, &size) == -1)
                    {
                        perror("getsockopt");
                        free_close_exit(socket_fd, epfd, 1);
                    }
                    if (errflag != 0)
                    {
                        perror("Failed to Connect");
                        free_close_exit(socket_fd, epfd, 1);
                    }
                    printf("Sucess to Connect. Input Message and enter\n");
                    connected = 1;
                }
                else if (!sent && strlen(buf) > 0)
                {
                    send(socket_fd, buf, strlen(buf), 0);
                    printf("Message Go: %s\n", buf);
                    sent = 1;
                    
                    ev.events = EPOLLIN;
                    ev.data.fd = socket_fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, socket_fd, &ev);
                }
            }
            else if (events[i].data.fd == STDIN_FILENO)
            {
                memset(buf, 0, 256);
                if (fgets(buf, 256, stdin) == NULL)
                    free_close_exit(socket_fd, epfd, 1);
                buf[strcspn(buf, "\n")] = '\0';
                    
                    
                if (connected) 
                {
                    ev.events = EPOLLOUT;
                    ev.data.fd = socket_fd;
                    epoll_ctl(epfd, EPOLL_CTL_MOD, socket_fd, &ev);
                    sent = 0;
                }
            }
        }
    }
}
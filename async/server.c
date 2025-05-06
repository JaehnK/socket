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

int open_socket()
{
    int socket_fd;

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        return (-1);
    }
    printf("socket fd: %d\n", socket_fd);
    return (socket_fd);
}

int init_nonblock_socket(int socket_fd)
{
    int flags, opt;

    opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        return (-1);
    }

    flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    return (0);
}

int bind_and_listen(int socket_fd, struct sockaddr_in *socket_addr)
{
    memset(socket_addr, 0, sizeof(struct sockaddr_in));
    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(8080);
    socket_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_fd, (struct sockaddr *)socket_addr, sizeof(*socket_addr)) == -1)
    {
        perror("bind");
        return (-1);
    }

    if (listen(socket_fd, SOMAXCONN) == -1)
    {
        perror("listen");
        return (-1);
    }
    printf("listening ...\n");
    return (0);
}

int init_epoll(int socket_fd, struct epoll_event *ev)
{
    int epfd;

    epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create");
        return (-1);
    }
    
    ev->events = EPOLLIN;
    ev->data.fd = socket_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, ev) == -1)
    {
        perror("epoll_ctl");
        return(-1);
    }

    return (epfd);
}

int main()
{
    int                 socket_fd, epfd, nfds;
    char                buf[256];
    struct sockaddr_in  socket_addr;
    struct epoll_event  ev, events[MAX_EVENTS];
    
    socket_fd = open_socket();
    if (socket_fd == -1)
        return (-1);
    
    if (init_nonblock_socket(socket_fd) == -1)
        return (-1);
    
    if (bind_and_listen(socket_fd, &socket_addr) == -1)
        return (-1);
    
    if ((epfd = init_epoll(socket_fd, &ev)) == -1)
        return (-1);


    struct sockaddr_in  client_addr;
    socklen_t           addr_size;
    int                 client_fd, flag;

    while (1)
    {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            break ;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == socket_fd)
            {
                addr_size = sizeof(client_addr);
                client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &addr_size);
                if (client_fd == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue ;
                    else
                    {
                        perror("accept");
                        continue ;
                    }
                }

                flag = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);

                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                printf("New Client Connect: %s %d (fd: %d)\n", \
                        client_ip, ntohs(client_addr.sin_port), client_fd);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
                {
                    perror("epoll_ctl: client fd");
                    close(client_fd);
                    continue ;
                }
            }
            else
            {
                int bytes;
                client_fd = events[i].data.fd;
                memset(buf, 0, 255);
                bytes = recv(client_fd, buf, 255, 0);
                if (bytes <= 0)
                {
                    if (bytes == 0) 
                        printf("클라이언트 연결 종료 (fd: %d)\n", client_fd);
                    else
                        perror("recv");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                    continue ;
                }
                printf("클라이언트 (fd: %d)로부터 수신: %s\n", client_fd, buf);
                // 에코 서버로 작동하려면 다음 코드 추가
                send(client_fd, buf, bytes, 0);
                
                // 응답 후 연결 유지하지 않고 바로 종료하려면
                // printf("클라이언트 연결 종료 (fd: %d)\n", client_fd);
                // epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
                // close(client_fd);
            }

        }
    }
  
    close(epfd);
    close(socket_fd);
    return (0);
}
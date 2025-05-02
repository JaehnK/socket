#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <memory.h>




int main()
{
    int                 socket_fd;
    int                 accept_fd;
    unsigned int        addr_size;
    struct sockaddr_in  *socket_addr;
    struct sockaddr_in  *client_addr;
    char                buf[256];
    
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        write(1, "socket fd\n", 11);
        return (-1);
    }
    
    printf("socket: %d\n", socket_fd);
    
    socket_addr = calloc(sizeof(struct sockaddr_in), 1);
    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(8080);
    socket_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(socket_fd, (struct sockaddr *)socket_addr, sizeof(*socket_addr)) < 0)
    {
        write(1, "bind\n", 5);
        return (-1);
    }
    
    if (listen(socket_fd, 256) < 0)
    {
        write(1, "listen\n", 8);
        return (-1);
    }

    printf("Listening ...\n");

    client_addr = calloc(sizeof(struct sockaddr_in), 1);
    addr_size = sizeof(client_addr);
    accept_fd = accept(socket_fd, (struct sockaddr *)client_addr, &addr_size);
    if (accept_fd < 0)
    {
        write(1, "accept\n", 8);
        return (-1);
    }
    
    memset(buf, 0, sizeof(buf));
    if (recv(accept_fd, buf, sizeof(buf) - 1, 0))
        printf("Received: %s\n", buf);
    
    close(accept_fd);
    close(socket_fd);

    return (0);
}
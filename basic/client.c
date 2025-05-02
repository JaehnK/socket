#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int open_socket()
{
    int socket_fd;

    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("socket_fd: ");
    }
    return (socket_fd);
}

int alloc_addr_and_connect(struct sockaddr_in *socket_addr, int socket_fd)
{
    socket_addr = calloc(sizeof(struct sockaddr_in), 1);
    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &socket_addr->sin_addr) <= 0)
    {
        perror("inet_pton: ");
    }
    
    if (connect(socket_fd, (struct sockaddr *)socket_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect:");
    }
    return (0);
}

int main(void)
{
    int                 socket_fd;
    struct sockaddr_in *socket_addr;
    char                buf[256];

    socket_fd = open_socket();
    
    alloc_addr_and_connect(socket_addr, socket_fd);
      
    printf("Waiting to send: ");
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0'; 
    send(socket_fd, buf, strlen(buf), 0);

    free(socket_addr);
    close(socket_fd);
}
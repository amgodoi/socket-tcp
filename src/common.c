/**
 * common.c
 */
#include "common.h"
#include <stdlib.h>

// Usage
void print_usage(const char *name)
{
    printf("Usage: %s [address] [port]\n", name);
}


void *get_sockaddr_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

uint16_t get_sockaddr_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return ((struct sockaddr_in*)sa)->sin_port;
    }

    return ((struct sockaddr_in6*)sa)->sin6_port;
}

int enable_reuse_addr(int sockfd)
{
    int flag = 1;

    // avoid "Address already in use" error message 
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
}


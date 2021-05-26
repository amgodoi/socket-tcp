/**
 * common.h
 */
#pragma once

#include <stdio.h>
#include <inttypes.h>

//#include <sys/socket.h>
#include <netinet/in.h>
//#include <netdb.h>
//#include <arpa/inet.h>

// Usage
void print_usage(const char *name);

// sockaddr_in or sockaddr_in6 address - IPv4 or IPv6
void *get_sockaddr_addr(struct sockaddr *sa);

// sockaddr_in or sockaddr_in6 port - IPv4 or IPv6
uint16_t get_sockaddr_port(struct sockaddr *sa);

// avoid "Address already in use" error message 
int enable_reuse_addr(int sockfd);


/**
 * client.c
 */
//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <inttypes.h> // C99 standard, needed to use printf and scanf with <stdint.h> data types
//#include <stdint.h>   // C99 standard
#include <stdbool.h>  // C99 standard
//#include <wchar.h>    // wchar_t definition
//#include <stddef.h>   // wchar_t definition

//#include <locale.h>

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "common.h"

#define MAXDATASIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3) {
        print_usage(argv[0]);
        exit(1);
    }

    const char *address = argv[1];
    const char *port = argv[2];
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((ret = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "client: (getaddrinfo): %s\n", gai_strerror(ret));
        exit(1);
    }
    
    printf("client: getaddrinfo\n");
    
    bool is_connected = false;
    int sockfd;

    // loop through the results and bind to the first one that we can
    for(struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: (socket)");
            continue;
        }
        
        printf("client: socket %d\n", sockfd);
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: (connect)");
            continue;
        }
        
        // socket created and connect ... then print info and break
        char ipstr[INET6_ADDRSTRLEN];
        uint16_t ipport;

        inet_ntop(p->ai_family, get_sockaddr_addr(p->ai_addr), ipstr, sizeof(ipstr));
        ipport = ntohs(get_sockaddr_port(p->ai_addr));
        printf("client: connected %s %u\n", ipstr, ipport);
    
        is_connected = true;
        
        break;
    }
    
    freeaddrinfo(servinfo);

    if (!is_connected) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    
    int numbytes;
    char buf[MAXDATASIZE];

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        close(sockfd);
        perror("client: (recv)");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n", buf);
    printf("client: received '%d bytes'\n", numbytes);

    close(sockfd);

    return 0;
}


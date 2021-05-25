/**
 * server.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <inttypes.h>   // C99 standard
//#include <stdint.h>   // C99 standard
#include <stdbool.h>    // C99 standard
//#include <wchar.h>    // wchar_t definition
//#include <stddef.h>   // wchar_t definition

#include <errno.h>

#include <signal.h>
#include <sys/wait.h>

//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"

#define LISTEN_BACKLOG 20

// Usage
void print_usage(const char *name)
{
    printf("Usage: %s [address] [port]\n", name);
}

// SIGCHLD handler
void sigchld_handler(int signal)
{
    char *signal_name = strdup(strsignal(signal));
    
    printf("sigchld_handler - signal: %d (%s)\n", signal, signal_name);

    free(signal_name);

    // waitpid() overwrite errno, so save and restore
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

int enable_remove_dead_process()
{
    struct sigaction sa;

    // remove all dead processes
    sa.sa_handler = sigchld_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    return sigaction(SIGCHLD, &sa, NULL);
}

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
    //hints.ai_flags = AI_PASSIVE;

    if ((ret = getaddrinfo(address, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "server: (getaddrinfo): %s\n", gai_strerror(ret));
        exit(1);
    }
        
    printf("server: getaddrinfo\n");
    
    bool is_bound = false;
    int sockfd;

    // loop through the results and bind to the first one that we can
    for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: (socket)");
            continue;
        }

        printf("server: socket %d\n", sockfd);

        // avoid "Address already in use" error message 
        if (enable_reuse_addr(sockfd) == -1) {
            perror("server: (setsockopt)");
            exit(1);
        }
        
        printf("server: setsockopt SO_REUSEADDR yes\n");

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: (bind)");
            continue;
        }

        // socket created and bound ... then print info and break
        char ipstr[INET6_ADDRSTRLEN];
        uint16_t ipport;

        inet_ntop(p->ai_family, get_sockaddr_addr(p->ai_addr), ipstr, sizeof(ipstr));
        ipport = ntohs(get_sockaddr_port(p->ai_addr));
        printf("server: bind %s %u\n", ipstr, ipport);
    
        is_bound = true;
        
        break;
    }

    freeaddrinfo(servinfo);

    if (!is_bound) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
        

    if (listen(sockfd, LISTEN_BACKLOG) == -1) {
        perror("server: (listen)");
        exit(1);
    }

    printf("server: listen %u\n", LISTEN_BACKLOG);

    if (enable_remove_dead_process() == -1) {
        perror("server: (sigaction)");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof(their_addr);
        int client_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        
        if (client_fd == -1) {
            perror("server: (accept)");
            continue;
        }
        
        printf("server: accept ... socket %d\n", client_fd);

        char client_address[INET6_ADDRSTRLEN];
        uint16_t client_port;

        inet_ntop(their_addr.ss_family, get_sockaddr_addr((struct sockaddr *)&their_addr), client_address, sizeof(client_address));
        client_port = ntohs(get_sockaddr_port((struct sockaddr *)&their_addr));
        printf("server: got connection from %s %u\n", client_address, client_port);
        
        int ret;

        if ((ret = fork()) == 0) { 
            // this is the child process
            close(sockfd); // child doesn't need the listener
            
            char *msg = "Hello, world!\n";

            if (send(client_fd, msg, strlen(msg), 0) == -1) {
                perror("server: (send)");
            }

            close(client_fd);
            exit(0);
        } else if (ret > 0) {
            // this is the parent process
            close(client_fd); // parent doesn't need this
            continue;
        } else {
            // this is the parent process with fork error
            perror("server: (fork)");
            close(client_fd); // parent doesn't need this
            exit(1);
        }
    }

    return 0;
}


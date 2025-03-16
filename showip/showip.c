#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char* argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "usage: showip [domain]\n");
        exit(1);
    }

    printf("IP addresses for %s\n\n", argv[1]);

    struct addrinfo hints;
    struct addrinfo* servinfo;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(argv[1], NULL, &hints, &servinfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    }

    struct addrinfo* p;
    char ip[INET6_ADDRSTRLEN];
    char* type;
    memset(ip, 0, sizeof(ip));

    void* addr;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {  // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in*) p->ai_addr;
            addr = &(ipv4->sin_addr);
            type = "IPv4";
        }
        if (p->ai_family == AF_INET6) {     // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6*) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            type = "IPv6";
        }
        inet_ntop(p->ai_family, addr, ip, sizeof(ip));
        printf(" %s: %s\n", type, ip);
    }

    freeaddrinfo(servinfo);
    return 0;
}
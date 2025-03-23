#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "3490"     // The port on which the server accepts connection request
#define MAX_DATA_SIZE 100   // max number of bytes we can get at once

void *get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv)
{
    struct addrinfo hints, *servinfo, *p;
    int rv, numbytes;
    int sockfd;
    char s[INET6_ADDRSTRLEN];
    char buf[MAX_DATA_SIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (argc != 2) {
        fprintf(stderr, "usage: client.exe [server ip/domain]\n");
        return 1;
    }

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo ; p != NULL ; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, 
            p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }

        if((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    if((numbytes = recv(sockfd, buf, MAX_DATA_SIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);

    // let's try sending an acknowledgement msg
    if((rv = send(sockfd, "ACK", 3, 0)) == -1) {
        perror("client: send");
        exit(1);
    }

    printf("client: sent %d bytes\n", rv);
   close(sockfd);

    return 0;
}

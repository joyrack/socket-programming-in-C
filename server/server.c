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

#define PORT "3490"
#define BACKLOG 10

void sigchld_handler(int s) {
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
    struct addrinfo hints, *servinfo, *p;
    int sockfd, new_fd;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;    // stream socket
    hints.ai_flags = AI_PASSIVE;        // use my ip

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo ; p != NULL ; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    // while (1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1) {
            perror("accept");
            return 1;
//            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), 
        s, sizeof s);
        printf("server: got connection from %s\n", s);

        // if (!fork()) {
            close(sockfd);  // close the original fd because we are no longer accepting any new connections

            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
//            close(new_fd);
  //          exit(0);
        // }
        printf("server: message sent!\n");
        char data[100];
        memset(data, 0, sizeof data);

        if((rv = recv(new_fd, data, 99, 0)) == -1) {
            perror("server: recv");
            exit(1);
        }
        
        printf("server: received %d bytes from client. Msg {%s}\n", rv, data);

        close(new_fd);  // close it - we only wanted to send one message
        //break;
    //}

    return 0;
}
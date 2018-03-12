#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define failif(x, msg) do {if (x) {perror(msg); exit(1);}} while(0);
#define BUF_LEN 64
#define NOBODY 65534
#define REPLY_CONST " : USERID : UNIX : "
#define REPLY_CONST_LEN 19

const char *ident;
int ident_len;

void *reply(void *args) {
    int r, rt;
    char buf[BUF_LEN];
    int fd;
    struct timeval tv;

    /* get arguments */
    if (!args) return NULL;
    fd = *((int *)args);
    free(args);

    /* set up timeout of 5s */
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    /* read data */
    r = 0;
    while (r < BUF_LEN) {
		/* read a bunch of bytes */
		if ((rt = read(fd, buf + r, BUF_LEN - r)) <= 0) { close(fd); return NULL; }
		r += rt;

        /* check end of line */
		if (buf[rt-1] == '\n') break;
	}

    /* too many bytes sent */
    if (r >= BUF_LEN) { close(fd); return NULL; }

    /* invalid number of bytes sent (too few) */
    if (r < 3) { close(fd); return NULL; }

    /* check the length will be ok (64 chars is a reasonable nunber) */
    if (r + ident_len + REPLY_CONST_LEN > BUF_LEN) { printf("reply too big\n"); close(fd); return NULL; }

    /* overwrite the \r\n */
    r -= 2;

    /* r is the end of the question; copy the constant part of the answer */
    memcpy(buf + r, REPLY_CONST, REPLY_CONST_LEN);
    r += REPLY_CONST_LEN;

    /* now the ident itself */
    memcpy(buf + r, ident, ident_len);
    r += ident_len;

    /* now set the \r\n */
    buf[r++] = '\r';
    buf[r++] = '\n';
    buf[r] = '\0'; /* not written to */

    /* write the values back */
    write(fd, buf, r);
    close(fd);
    return NULL;
}

int main(int argc, char **argv) {
    int sock, csock, *c;
    struct sockaddr_in6 server;
    pthread_attr_t attr;
    pthread_t thread;

    /* figure out the ident */
    if (argc == 2) {
        /* try args */
        ident = argv[1];
    } else if ((ident = getenv("IDENT")) == NULL) { /* try env vars */
        /* placeholder */
        ident = "root";
    }
    ident_len = strlen(ident);
    printf("Using ident %s\n", ident);

    /* socket setup */
    sock = socket(AF_INET6, SOCK_STREAM, 0);
    failif(sock < 0, "Failed to create socket");
    server.sin6_family = AF_INET6;
    server.sin6_addr = in6addr_any;
    server.sin6_port = htons(113);

    /* bind */
    failif(
        bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0,
        "Failed to bind to socket"
    );

    /* listen */
    failif(
        listen(sock, 5) < 0,
        "Failed to start listening"
    );

    /* pthread config */
    failif(pthread_attr_init(&attr) != 0, "Failed to configure threads");
    failif(
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0,
        "Failed to configure threads"
    );

    /* become a safer user */
    setgid(NOBODY);
    setuid(NOBODY);

    /* start blocking for connections */
    while (1) {
        failif(
            (csock = accept(sock, NULL, NULL)) < 0,
            "Failed to accept() a connection"
        );

        if (!(c = malloc(sizeof(int)))) {
            close(csock);
            continue;
        }
        *c = csock;

        /* start thread + values */
        pthread_create(&thread, &attr, &reply, c);
    }

    return -1;
}

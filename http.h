#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "configs.h"
#include "common.h"

#define HTTP_ACCEPT "*/*"
#define CACHE_CONTROL "Cache-Control:max-age=0"
#define CONNECTION ""//"Connection:keep-alive"
struct url {
    char *full_url; //完整url
    char *host; //主机名
    char *uri;
    char *filename;
    char *dir;
};

struct header {
    int status;
    char *content_length;
    char *location;
    char *set_cookie;
};

void free_url(struct url *url);

void free_header(struct header *header);

int make_connection(char *hostname, int port);

int send_header(int sockfd, char *header_str);

int recv_resp(int sockfd, char *buf, int size);



char *parse_header(char *buf, int size, struct header *resp);


void header_print(struct header *resp);


char *header_create(char *method, char *uri);
char *header_set(char **pheader_str, char *key, char *value);
char *header_finish(char *header_str);
#endif

#include "http.h"

void free_url(struct url* url) {
    free(url->full_url);
    free(url->host);
    free(url->uri);
    free(url->filename);
    free(url->dir);
}

void free_header(struct header *header) {
    free(header->content_length); 
    free(header->location);
    free(header->set_cookie);
}
/*与hostname:port主机建立连接，返回sockfd*/
int make_connection(char *hostname, int port) {
    int sockfd;
    struct hostent *hostentry;
    struct sockaddr_in hostaddr;
    char str[100];

    debugp("Start to get host addr...\n");
    hostentry = gethostbyname(hostname);
    if (hostentry == NULL) {
        printf("!!!gethostbyname Error!\n");
        return -1;
    }
    debugp("Start to create a socket...\n");

    sockfd = socket(hostentry->h_addrtype, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("!!!Socket Error!\n");
        return -1;
    }


    debugp("Start to configure hostaddr structure...\n");
    memset(&hostaddr, 0, sizeof(hostaddr));
    memcpy(&hostaddr.sin_addr, hostentry->h_addr_list[0], hostentry->h_length);
    debugp("Start to test the address...\n");
    inet_ntop(hostentry->h_addrtype, &hostaddr.sin_addr.s_addr, str, sizeof(str));
    //printf("Address : %s\n", str);
    hostaddr.sin_family = hostentry->h_addrtype;
    hostaddr.sin_port = htons(port);
    
    debugp("Start to connect to the host...\n");
    if (connect(sockfd, (struct sockaddr *)&hostaddr, sizeof(hostaddr)) < 0) {
        printf("!!!connect Error!\n");
        return -1;
    }
    return sockfd;
}

/*发送请求头*/

int send_header(int sockfd, char *header_str) {
    return send(sockfd, header_str, strlen(header_str), 0);
}



char *header_create(char *method, char *uri) {
    int len;
    char *header_str;

    //       Method Uri HTTP/1.1\r\n\0  
    len = strlen(method) + 1 + strlen(uri) + 1 + 8 + 2 + 1 + 2;  //+2是为了预留空间给结尾的空行\r\n
    header_str = (char *)malloc(sizeof(char) * len);
    if (header_str == NULL) {
        return NULL;
    }
    sprintf(header_str, "%s %s HTTP/1.1\r\n", method, uri);
    return header_str;
}

char *header_set(char **pheader_str, char *key, char *value) {
    int old_len;
    char *new_header;
    old_len = strlen(*pheader_str);

    new_header = (char *)realloc(*pheader_str, sizeof(char) * (old_len + strlen(key) + 2 + strlen(value) + 3 + 2));//+2是为了预留空间给结尾的空行\r\n
    if (new_header == NULL) {
        return NULL;
    }
    sprintf(new_header, "%s%s: %s\r\n", *pheader_str, key, value);
    *pheader_str = new_header;
    return new_header;
}
    
char *header_finish(char *header_str) {
    int old_len;

    old_len = strlen(header_str);
    header_str[old_len ++] = '\r';
    header_str[old_len ++] = '\n'; 
    header_str[old_len] = '\0';
    return header_str;
}

   


/*读取响应*/

int recv_resp(int sockfd, char *buf, int size) {
    int res;

    res = recv(sockfd, buf, size, MSG_WAITALL);
    return res;
}

char *parse_header(char *buf, int size, struct header *resp) {
    char *pos = buf; 
    char *pos2;
    int tmp = 0; 
    int len;
    char **ppchr, *pchr;
    struct pair *info;
    
    memset(resp, 0, sizeof(struct header));

    pos += 9;
    while (isdigit(*pos)) {
        tmp = tmp * 10 + (*pos - '0');
        pos ++;
    }
    resp->status = tmp;
    pos = strchr(pos, '\n') + 1;
    while(strncmp(pos, "\r\n", 2) != 0) {
        if (pos == NULL || pos == buf + size) {
            return NULL;
        } else { 
            pos2 = strchr(pos, ':'); 
            len = pos2 - pos + 1;
            if (strncasecmp(pos, "Content-Length", 14) == 0) { 
                ppchr = &(resp->content_length);
            } else if (strncasecmp(pos, "Location", 8) == 0) { 
                ppchr = &(resp->location);
            } else if (strncasecmp(pos, "Set-Cookie", 10) == 0) {
                ppchr = &(resp->set_cookie);
            } else if (strncasecmp(pos, "Content-Disposition", 19) == 0){
                ppchr = &(resp->content_disposition);  
            } else {
                ppchr = NULL;
            }
            pos = pos2 + 1;
            while ( (*pos) == ' ') { 
                pos ++;
            }
            pos2 = strchr(pos, '\r'); 
            len = pos2 - pos + 1;
            if (ppchr != NULL) {
                if (*ppchr != NULL) { //已有
                    tmp = strlen(*ppchr);
                    *ppchr = (char *)realloc(*ppchr, sizeof(char) * (tmp + len + 2));
                    pchr = *ppchr + tmp;
                    *(pchr ++) = ';'; 
                    *(pchr ++) = ' ';
                } else {
                    *ppchr = (char *)malloc(sizeof(char) * len);
                    pchr = *ppchr;
                }
                strncpy(pchr, pos, len);
                pchr[len - 1] = 0;
            } 
            pos = pos2 + 2; 
        }   
    }
    return pos + 2; 
}

#ifdef DEBUG
void header_print(struct header *resp) {
    printf("Status : %d\n", resp->status);
    if (resp->content_length)  printf("Content-Length : %s\n", resp->content_length);
    if (resp->location) printf("Location : %s\n", resp->location);
    if (resp->set_cookie) printf("Set-Cookie : %s\n", resp->set_cookie);
}
#endif


int parse_url(char *url, struct url *aurl) {
    char *pos = url;
    char *pos2, *pos3;
    char *host;
    int n;

    //去除开头的http://
    if (strncasecmp(pos, "http://", 7) == 0) {
        pos += 7;
    }

    //host
    pos2 = strchr(pos, '/');
    if (!pos2) return -1;
    n = pos2 - pos;
    host = (char *)malloc(sizeof(char) * (n + 1));
    strncpy(host, pos, n);
    host[n] = '\0';
    aurl->host = host;

    //uri
    pos = pos2;
    n = strlen(pos) + 1;
    aurl->uri = (char *)malloc(sizeof(char) * n);
    strcpy(aurl->uri, pos);

    //filename
    pos2 = strrchr(pos, '/') + 1;
    pos3 = strchr(pos2, '?');
    if (pos3 == NULL) {
        pos3 = strrchr(pos2, 0);
    }
    n = pos3 - pos2 + 1;
    aurl->filename = (char *)malloc(sizeof(char) * n);
    strncpy(aurl->filename, pos2, n - 1);
    aurl->filename[n - 1] = 0;
    
    //dir
    n = pos2 - pos;
    aurl->dir = (char *)malloc(sizeof(char) * (n + 1));
    strncpy(aurl->dir, pos, n);
    aurl->dir[n] = '\0';
    return 0;
}

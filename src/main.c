#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>


#include "http.h"
#include "bitmap.h"
#include "cookie.h"
//extern int h_errno;

int download(char *url, char *saveto); 
void usage(char *prog);

struct cookies cookies;



int main(int argc, char **argv) {
    char *url = NULL;
    char buf[MAX_COOKIE_LEN];
    memset(&cookies, 0, sizeof(struct cookies));
    int index = 1;
    char c;
    
    if (argc < 2) {
        printf("Wrong Usage!\n\n");
        usage(argv[0]);
        return 0;
    }
    while (index < argc) {
        switch (*(argv[index])) {
            case '-':
                c = *(argv[index] + 1);
                switch (c) {
                    case 'c':  //c参数 cookies设置  
                        index ++;
                        cookies_set(&cookies, argv[index]); //TODO: 测试参数 -c a=b之后，
                                                           //生成的cookie_str居然是"a=b; a=b"
                                                           //亟待解决
                        break;
                    default:
                        break;
                }
                break;
            default:
                if (url == NULL) {
                    url = argv[index];
                } else {
                    usage(argv[0]);
                    return 0;
                }
                break;
        }
        index ++;
    } 
    download(url, "");
    return 0;
}

void usage(char *prog) {
    printf("\n"
            "-----Dingloader  " VERSION_STRING " -----\n" 
            "A lite and simple and dump  downloader.\n"
            "usage : %s [-c cookies] URL\n\n"
            "\t-c cookies       Set the cookies\n"
            "\n", prog
          );
}
int download(char *url, char *saveto) {
    char buf[2896];
    int sockfd;
    int response_size;
    long downloaded = 0;
    FILE *fp;
    long size;
    char c;
    char *header;
    char *request_header_str;
    char *content;
    struct header resp;
    int header_len;
    struct url_info_s aurl;
    struct timeval tv1, tv2;
    long double usec;
    long double speed;  //TODO: 下载速度完全不对,请修正
    char *pchr;
    char cookie_str[MAX_COOKIE_LEN];
    

    debugp("URL : %s\n", url);
    parse_url(url, &aurl);

    debugp("host: %s\n", aurl.host);
    sockfd = make_connection(aurl.host, 80);
    if (sockfd < 0) {
        return -1;
    }

#ifdef DEBUG  //request for the header only
    if(aurl.filename == NULL || strlen(aurl.filename) == 0) {
        debugp("Starting to configure the request header...\n");
        request_header_str = header_create("HEAD", aurl.uri);
        header_set(&request_header_str, "Accept", "*/*");
        //header_set(&request_header_str, "Connection", "close");
        header_set(&request_header_str, "Connection", "keep-alive");
        header_set(&request_header_str, "Host", aurl.host);
        header_set(&request_header_str, "Cache-Control", "max-age=0");
        header_finish(request_header_str);

        debugp("request_header_str: \"%s\"\n", request_header_str);

        debugp("Starting to send the request header...\n"); 
        send_header(sockfd, request_header_str); 
        header_len = recv_resp(sockfd, buf, sizeof(buf));
        buf[header_len] = 0;
        printf("responsed header length: %d\n", header_len);
        printf("responsed Header content:\n\"%s\"\n\n\n", buf);
        parse_header(buf, header_len, &resp);
        if (resp.content_disposition) {
            if (strncmp(resp.content_disposition, "attachment", 10) == 0) {
                pchr = strchr(resp.content_disposition, '='); 
                if (pchr) {
                    saveto = malloc(strlen(pchr) * sizeof(char));
                    strcpy(saveto, pchr + 1);
                }
            }
        }
        free(request_header_str);
        free_header(&resp);
        return 0;
    } else {
        saveto = aurl.filename;
    }
#endif




//设置请求头 
    debugp("Starting to configure the request header...\n");
    request_header_str = header_create("GET", aurl.uri);
    header_set(&request_header_str, "Accept", "*/*");
    header_set(&request_header_str, "Connection", "close");
    header_set(&request_header_str, "Host", aurl.host);
    header_set(&request_header_str, "Cache-Control", "max-age=0");
    if (cookies.cookie_list != NULL) {
        debugp("Found cookies from previous request, adding to the header...\n");
        //cookies_print(&cookies);
        cookies_get(&cookies, cookie_str);
        header_set(&request_header_str, "Cookie", cookie_str);
        debugp("Cookies Str : (%s)\n", cookie_str);  
    }
    header_finish(request_header_str);

    debugp("Starting to send the request header...\n"); 
    send_header(sockfd, request_header_str); 
    //saveto = aurl.filename;

    fp = fopen(saveto, "w");
    if (fp == NULL) {
        printf("Cannot open file %s.\n", saveto);
        return -1;
    }
    gettimeofday(&tv1, NULL);
    response_size = recv_resp(sockfd, buf, sizeof(buf));
    if (response_size < 0) {
        printf("Error!");
        return -1;
    }
    gettimeofday(&tv2, NULL);
    usec = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
    content = parse_header(buf, strlen(buf), &resp); 
    debugp("Status : %d\n", resp.status);

#ifdef  DEBUG//print header
    header_len = content - buf;
    debugp("\n\n\nHeader Length : %d\n", header_len);
    header = (char *)malloc(sizeof(char) * (header_len + 1));
    strncpy(header, buf, header_len); 
    header[header_len] = '\0';
    debugp("Response Header :\n%s\n", header);
#endif

   if (resp.set_cookie) {
        cookies_set(&cookies, resp.set_cookie);
    }
    
    if (resp.status == 301 || resp.status == 302 || resp.status == 303 || resp.status == 307) {
        close(sockfd);
        debugp("Redirect to %s\n", resp.location);
        download(resp.location, saveto);
        free_url(&aurl);
        free_header(&resp);
        return 0;
    }
    if (resp.status == 404) {
        debugp("URL does not exist!\n");
        close(sockfd);
        free_url(&aurl);
        free_header(&resp);
        return -1;
    }


    if (resp.status != 200) return -1;
    header_len = content - buf;
    downloaded = response_size - header_len;
    if (resp.content_length == NULL && downloaded == 0) {
        close(sockfd);
        debugp("No Content-Length, Sending a requset again...");
        download(url, saveto);
        free_url(&aurl);
        free_header(&resp);
        return 0;
    }
    if (resp.content_length) size = atol(resp.content_length);
    else size = 0;
    speed = (long double)downloaded * 1000 / usec ;
    fwrite(content, downloaded, sizeof(char), fp);
    printf("\nDownloading %ld/%ld\t\t%.2LfkB/s", downloaded, size, speed);

    gettimeofday(&tv1, NULL);
    while((response_size = recv_resp(sockfd, buf, sizeof(buf))) > 0) {
        gettimeofday(&tv2, NULL);
        usec = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
        downloaded += response_size;
        speed = (long double)response_size * 1000 / usec ;
        printf( "\rDownloading %ld/%ld\t\t%.2LfkB/s       ", downloaded, size, speed);
        fflush(stdout);
        fwrite(buf, response_size, sizeof(char), fp); 
        gettimeofday(&tv1, NULL);
    }

    printf("\n");

    //清理
    fclose(fp); 
    close(sockfd);
    free(header);
    free_header(&resp);
    free_url(&aurl);
    return size;
}

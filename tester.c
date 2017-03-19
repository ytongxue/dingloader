#include <stdio.h>
#include <strings.h>
#include "http.h"

int main(int argc, char **argv) {
    struct url  url_info;
    char *cases[] = {"http://baidu.com",};
    for (int i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        char *acase = cases[i];
        printf("case: %s\n", acase);
        bzero(&url_info, sizeof(url_info));
        parse_url(acase, &url_info);
        if (url_info.host) {
            printf("host: %s\n", url_info.host);
        } else {
            printf("host is NULL\n");
        }
    }
    return 0;
}

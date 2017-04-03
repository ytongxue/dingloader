#include "cookie.h"
#include <strings.h>
#include <ctype.h>

static int countChr(char *str, unsigned char c);



void free_cookies(struct cookie_jar_s *cookies) {
    struct cookie *tmp, *tmp2;

    free(cookies->host);
    free(cookies->path);
    free(cookies->domain);
    free(cookies->expires);
    tmp = cookies->cookie_list;
    while (tmp != NULL) {
        tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    }
}


void cookies_set(struct cookie_jar_s *cookies, char *set_cookie) {
    char *pos = set_cookie;
    char *pos2, *pos3;
    char c;
    int n;
    int i;
    int name_len;
    int val_len;
    char name[MAX_COOKIE_NAME_LEN] = {0};  
    struct cookie *cookie;
    
    n = countChr(set_cookie, ';') + 1; 

    for (i = 0; i < n; i ++) {
        pos2 = strchr(pos, '=');
        if (pos2 == NULL) {
            //debugp("[%s:%d] what the fuck??\n", __FUNCTION__, __LINE__);
            //debugp("set_cookie: \"%s\"\n", set_cookie);
            break;
        }
        name_len = pos2 - pos;
        if (name_len < 0) {
            debugp("[%s:%d] what the fuck??\n", __FUNCTION__, __LINE__);
            break;
        }
        pos3 = strchr(pos, ';'); 
        if (pos3 == NULL) {
            pos3 = strchr(set_cookie, '\0');
        }
        val_len = pos3 - pos2 - 1;
        cookie = cookies->cookie_list; 
        if (strncmp(pos, "expires", 7) == 0) goto jumpout;
        while (cookie != NULL) {
            strncpy(name, pos, name_len);
            name[name_len] = '\0';
            //查看是否已有此项
            if (strcmp(cookie->name, name) == 0) {   //存在则修改
                strncpy(cookie->value, pos2 + 1, val_len);
                cookie->value[val_len] = '\0';
                break;
            } 
            cookie = cookie->next;
        } 
        if (cookie == NULL) {  //未存在则保存
           cookie = (struct cookie *)malloc(sizeof(struct cookie)); 
           strncpy(cookie->name, pos, name_len);
           cookie->name[name_len] = '\0';
           strncpy(cookie->value, pos2 + 1, val_len);
           cookie->value[val_len] = '\0';
           cookie->next = cookies->cookie_list;
           cookies->cookie_list = cookie;
        }
jumpout:   //terrible idea
        pos = pos3 + 1; 
        while (*pos != '\0' && isblank(*pos)) {
            pos ++;
        }
    }
}



/*统计一个字符串str中字符c出现的次数*/
static int countChr(char *str, unsigned char c) {
    int i;
    int count = 0;
    int len = strlen(str);
    for (i = 0; i < len; i ++) {
        if (str[i] == c) {
            count ++;
        }
    }
    return count;
}


//将cookies结构体中的cookie转化为字符串，以供发送

void cookies_get(struct cookie_jar_s *cookies, char *buf) {
    struct cookie *cookie;
    char tmp[MAX_COOKIE_NAME_LEN + MAX_COOKIE_VAL_LEN + 3] = "";
    cookie = cookies->cookie_list;
    buf[0] = 0;
    while (cookie != NULL) {
        sprintf(tmp, "%s=%s; ", cookie->name, cookie->value);
        strcat(buf, tmp);
        cookie = cookie->next;
    }
    buf[strlen(buf) - 2] = '\0';
}


#ifdef DEBUG
void cookies_print(struct cookie_jar_s *cookies) {
    struct cookie *cookie;
    cookie = cookies->cookie_list;
    while (cookie != NULL) {
        printf("%s = %s\n", cookie->name, cookie->value);
        cookie = cookie->next;
    }
}
#endif

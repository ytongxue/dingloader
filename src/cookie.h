#ifndef COOKIE_H
#define COOKIE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "configs.h"
#include "common.h"

#define MAX_COOKIE_NAME_LEN     32 
#define MAX_COOKIE_VAL_LEN      128
#define MAX_COOKIE_LEN          4096


// cookie 链表
struct cookie {                      
    char name[MAX_COOKIE_NAME_LEN];
    char value[MAX_COOKIE_VAL_LEN];
    struct cookie *next;
};

struct cookie_jar_s {
    char *host;
    char *path;
    char *domain;
    char *expires;
    struct cookie *cookie_list;  //cookies链表
};

#ifdef DEBUG
void free_cookie(struct cookie_jar_s *cookies);
#endif

void cookies_set(struct cookie_jar_s *cookies, char *set_cookie);

void cookies_get(struct cookie_jar_s *cookies, char *buf);

void cookies_print(struct cookie_jar_s *cookies);

#endif

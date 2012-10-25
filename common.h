#ifndef COMMON_H
#define COMMON_H

#include "configs.h"

/*
#ifdef DEBUG
#define debugp(msg) printf(msg)
#else
#define debugp(msg)
#endif
*/
#ifdef DEBUG
#define debugp(...)  printf(__VA_ARGS__)
#else
#define debugp(msg)
#endif

#endif

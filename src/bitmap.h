#ifndef BITMAP_H
#define BITMAP_H
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int bitmap_checkExist(char *fileName);

int bitmap_create(FILE *fp,  unsigned long size);

int bitmap_setbit(FILE *fp,  unsigned long pos);

int bitmap_clrbit(FILE *fp,  unsigned long pos);

int bitmap_getbit(FILE *fp,  unsigned long pos);

#endif

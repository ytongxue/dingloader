/*
 *   create a bitmap file. filesize = size / 8 / blocksize
 *   Do remember to open the file with "rw" mode
*/
#include "bitmap.h"
extern int errno;

/* Test whether the file exists, 0 means yes and otherwise no */ 
int bitmap_checkExist(char *fileName) {
    if (access(fileName, F_OK) == 0) { 
        return 0;
    } else if (errno == ENOENT) {
        return -1;
    } else {
        return 1;
    }
}


#define WRITE_BLOCK_SIZE    4096
int bitmap_create(FILE *fp, unsigned long size) {
    char buf[WRITE_BLOCK_SIZE] = {0};
    unsigned long remain;
    unsigned long write_size;

    remain = size; 
    while (remain > 0) {
        if (remain > WRITE_BLOCK_SIZE) {
            write_size = WRITE_BLOCK_SIZE; 
        } else {
            write_size = remain;
        } 
        fwrite(buf, sizeof(char), write_size, fp);
        remain -= write_size;
    }
    //fflush(fp);
    return 0;
}




/*  set a bit on of the bitmap file */
int bitmap_setbit(FILE *fp, unsigned long pos) { 
    unsigned long byte_pos;
    unsigned short bit_pos;
    unsigned char byte;
    int result;

    byte_pos = pos >> 3;  //dived by 8
    bit_pos = pos & 7;  
    //fseek(fp, byte_pos, SEEK_SET);
    if (fseek(fp, byte_pos, SEEK_SET) < 0) {
        printf("!!!fsetpos Error!");
        return -1;
    }
    result = fgetc(fp); //read the original byte
    if (result < 0) {
        printf("!!!fgetc Error!\n");
        return -1;
    }
    byte = result;
    byte = byte | (1 << bit_pos);  //set the bit 
    //if (fsetpos(fp, &pos) < 0) {
    if (fseek(fp, byte_pos, SEEK_SET) < 0) {
        printf("!!!fsetpos Error!");
        return -1;
    }

    result = fputc(byte, fp);//and write the byte back 
    //fflush(fp);
    return result;
}



/* clear a bit of the bitmap file */
int bitmap_clrbit(FILE *fp, unsigned long pos) {
    unsigned long byte_pos;
    unsigned short bit_pos;
    unsigned char byte;

    byte_pos = pos >> 3;  //dived by 8
    bit_pos = pos & 7;  
    fseek(fp, byte_pos, SEEK_SET);
    byte = fgetc(fp); //read the original byte
    byte = byte & (~(1 << bit_pos));  //clear the bit 
    fseek(fp, byte_pos, SEEK_SET);  

    return fputc(byte, fp);//and write the byte back 
return 0;
}



/* get the status of a bit of the bitmap file, 0 for off, 1 for on */
int bitmap_getbit(FILE *fp, unsigned long pos) {
    unsigned long byte_pos;
    unsigned short bit_pos;
    unsigned char byte;
    int result;

    byte_pos = pos >> 3;  //dived by 8
    bit_pos = pos & 7;  
    fseek(fp, byte_pos, SEEK_SET);
    result = fgetc(fp); //read the original byte 
    if (result < 0) {
        printf("!!!fgetc Error!");
        return -1;
    }
    result = (result >> bit_pos) & 1; 
    return result;
}

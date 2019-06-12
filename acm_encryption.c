/*
 * Copyright (C), 2010-2017, ATCOM Tech. Co., Ltd.
 *
 * File Name: acm_encryption.c
 * Author: liaoxuecheng
 * mail: liaoxuecheng@atcom.com.cn
 * Created Time: Sun 05 May 2019 04:32:44 PM CST
 *
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <alloca.h>
#include "aes.h"
#include "encode_decode.h"

char *acm_encode(uint8_t*iv, uint8_t *key, const uint8_t *data, unsigned int len)
{
    uint8_t *in;
    char *encode_data = NULL;
    unsigned int real_len, in_len;
    struct AES_ctx ctx;

    real_len = len;
    in_len = (real_len + 15) / 16 * 16;

    if(in_len == real_len)
        in_len += 16;

    AES_init_ctx_iv(&ctx, key, iv);

    in = (uint8_t *)alloca(in_len);
    memcpy(in, data, real_len);
    memset(in + real_len, in_len - real_len, in_len - real_len);

    AES_CBC_encrypt_buffer(&ctx, in, in_len);

    encode_data = (char *)calloc(1, BASE64_LENGTH(in_len));
    assert(encode_data != NULL);
    base64_encode(in, in_len, encode_data);
    return encode_data;
}

uint8_t *acm_decode(uint8_t*iv, uint8_t *key, const char *in, unsigned int *out_len)
{
    int i;
    char *decode_data = NULL;
    unsigned int ret_len, strip_len, in_len;
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key, iv);
    in_len = strlen(in);
    
    decode_data = (char *)calloc(1, in_len);
    assert(decode_data != NULL);
    ret_len = base64_decode(in, decode_data);

    if(ret_len % 16)
    {
        free(decode_data);
        printf("%s:%d\n", __func__, __LINE__);
        return NULL;
    }

    AES_CBC_decrypt_buffer(&ctx, (uint8_t*)decode_data, ret_len);
    strip_len = *(decode_data + ret_len - 1);
    if(strip_len > 16)
    {
        free(decode_data);
        printf("%s:%d\n", __func__, __LINE__);
        return NULL;
    }

    for(i = 1; i <= strip_len; i++)
    {
        if(decode_data[ret_len-i] != strip_len)
        {
            free(decode_data);
            printf("%s:%d\n", __func__, __LINE__);
            return NULL;
        }
        else
            decode_data[ret_len-i] = '\0';
    }

    *out_len = ret_len - strip_len;

    return (uint8_t *)decode_data;
}

static int acm_sopen_file(const char *in, const char *out, int *fd, int *same)
{
    if(in == NULL || out == NULL)
    {
        printf("%s:%d %s %s\n", __func__, __LINE__, in, out);
        return -1;
    }
    if(strcmp(in, out) == 0)
        *same = 1;
    else
        *same = 0;
    if(*same)
    {
        fd[0] = fd[1] = open(in, O_RDWR);
    }
    else
    {
        fd[0] = open(in, O_RDONLY);
        fd[1] = open(out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
    if(fd[0] < 0 || fd[1] < 0)
    {
        printf("error: %s%d\n", __func__, __LINE__);
        close(fd[0]);
        if(*same == 0)
            close(fd[1]);
        return -1;
    }
    return 0;
}

static int acm_sread_file(int *fd, char *buf, int buf_len, const int same, int *read_flag)
{
    int real_len, in_len;

    real_len = read(fd[0], buf, buf_len);

    if(real_len < 0)
    {
        *read_flag = 0;
        return -1;
    }
    else if(real_len == 0)
    {
        *read_flag = 0;
        return 0;
    }
    else if(real_len < buf_len)
    {
        *read_flag = 0;
        return real_len;
    }
    return real_len;
}

static int acm_swrite_file(int *fd, char *buf, int buf_len, const int same)
{
    int ret;
    if(same)
    {
        lseek(fd[1], (same >> 8) - buf_len, SEEK_CUR);
    }
    ret = write(fd[1], buf, buf_len);
    return 0;
}
static int acm_sclose_file(int *fd, int *same)
{
    close(fd[0]);
    if(*same == 0)
        close(fd[1]);
    return 0;
}
int acm_encode_file(uint8_t*iv, uint8_t *key, const char *in, const char *out)
{
    int ret, read_flag = 1;
    int in_len;
    int fd[2];
    int is_same = 0;
    char buf[1024];
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    if(acm_sopen_file(in, out, fd, &is_same) < 0)
        return -1;
    do
    {
        ret = acm_sread_file(fd, buf, sizeof(buf), is_same, &read_flag);
        in_len = ret;
        if(read_flag == 0 && ret < sizeof(buf))
        {
            in_len = (ret + 15) / 16 * 16;
            if(in_len == ret)
            {
                in_len += 16;
            }
            memset(buf + ret, in_len - ret, in_len - ret);
            if(is_same)
                is_same |= (buf[ret]) << 8;
        }
        AES_CBC_encrypt_buffer(&ctx, (uint8_t*)buf, in_len);
        ret = acm_swrite_file(fd, buf, in_len, is_same);
    }while(read_flag);

    acm_sclose_file(fd, &is_same);
    return 0;
}

int acm_decode_file(uint8_t*iv, uint8_t *key, const char *in, const char *out)
{
    int ret, read_flag = 1;
    int fd[2];
    int is_same = 0;
    char buf[1024];
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);

    if(acm_sopen_file(in, out, fd, &is_same) < 0)
        return -1;

    struct stat stat_buf;
    fstat(fd[0], &stat_buf);
    uint32_t file_len = stat_buf.st_size;

    do
    {
        ret = acm_sread_file(fd, buf, sizeof(buf), is_same, &read_flag);
        AES_CBC_decrypt_buffer(&ctx, (uint8_t*)buf, ret);
        acm_swrite_file(fd, buf, ret, is_same);
    }while(read_flag);

    if(ret == 0)
    {
        ftruncate(fd[1], file_len-buf[sizeof(buf)-1]);
    }
    else if(0 < buf[ret-1] && buf[ret-1] <= 16)
        ftruncate(fd[1], file_len-buf[ret-1]);
    else
    {
        printf("decode file error %d\n", buf[ret-1]);
    }

    acm_sclose_file(fd, &is_same);
    return 0;
}

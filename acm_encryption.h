/*
 * Copyright (C), 2010-2017, ATCOM Tech. Co., Ltd.
 *
 * File Name: acm_encryption.h
 * Author: liaoxuecheng
 * mail: liaoxuecheng@atcom.com.cn
 * Created Time: Tue 07 May 2019 11:08:19 AM CST
 *
 */

#ifndef _ACM_ENCRYPTION_H_
#define _ACM_ENCRYPTION_H_

#include<stdint.h>

/* acm_encode and acm_decode use malloc, so you must free it ! */
char *acm_encode(uint8_t*iv, uint8_t *key, const uint8_t *data, unsigned int len);
uint8_t *acm_decode(uint8_t*iv, uint8_t *key, const char *in, unsigned int *out_len);
int acm_encode_file(uint8_t*iv, uint8_t *key, const char *in, const char *out);
int acm_decode_file(uint8_t*iv, uint8_t *key, const char *in, const char *out);

#endif //_ACM_ENCRYPTION_H_

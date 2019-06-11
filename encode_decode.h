#ifndef ENCODE_DECODE_H_
#define ENCODE_DECODE_H_
#define BASE64_LENGTH(len) (2 * (len))
size_t base64_encode (const void *data, size_t length, char *dest);
ssize_t base64_decode (const char *base64, void *dest);
#endif

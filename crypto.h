#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdbool.h>

#define SHA256_DIGEST_LENGTH 32
#define NONCE_LENGTH 6

char *base36enc(long unsigned int value);
unsigned char *longToBytes(unsigned long value);
unsigned long bytesToLong(unsigned char *value);
bool simpleSHA256(unsigned char *input, unsigned long length,
                  unsigned char *md);

#endif
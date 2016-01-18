#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "crypto.h"

#define SHA256_DIGEST_LENGTH 32
#define NONCE_LENGTH 6

// from http://stackoverflow.com/a/2262447
bool simpleSHA256(unsigned char *input, unsigned long length,
                  unsigned char *md) {
  SHA256_CTX context;

  if (!SHA256_Init(&context))
    return false;

  if (!SHA256_Update(&context, input, length))
    return false;

  if (!SHA256_Final(md, &context))
    return false;

  return true;
}

// from https://en.wikipedia.org/wiki/Base36
char *base36enc(long unsigned int value) {
  char base36[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
  /* log(2**64) / log(36) = 12.38 => max 13 char + '\0' */
  char buffer[14];
  unsigned int offset = sizeof(buffer);

  buffer[--offset] = '\0';
  do {
    buffer[--offset] = base36[value % 36];
  } while (value /= 36);

  return strdup(&buffer[offset]); // warning: this must be free-d by the user
}

unsigned char *longToBytes(unsigned long value) {   
  unsigned char *buffer = malloc(NONCE_LENGTH);

  for (int i = NONCE_LENGTH - 1; i >= 0; --i) {
    buffer[i] = value;
    value >>= CHAR_BIT;
  }

  return buffer; // warning: this must be free-d by the user
}

unsigned long bytesToLong(unsigned char *value) {
  unsigned long number = 0;
  
  for (int i = 0; i < NONCE_LENGTH; i++) {
    number <<= CHAR_BIT;
    number |= value[i];
  }
  
  return number;
}
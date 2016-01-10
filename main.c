#include <stdio.h>
#include <stdbool.h>

#include "curl.c"
#include "crypto.c"

#define DEBUG
#define NONCE_OFFSET 10000000

char *KRIST_SYNC_URL;
char *LAST_BLOCK_URL;
char *GET_WORK_URL;
char *GET_BALANCE_URL;

void init() {
  KRIST_SYNC_URL = httpGet("https://raw.githubusercontent.com/BTCTaras/"
                           "kristwallet/master/staticapi/syncNode");
  KRIST_SYNC_URL[strlen(KRIST_SYNC_URL) - 1] = '\0';

  LAST_BLOCK_URL = malloc(strlen(KRIST_SYNC_URL) + strlen("?lastblock") + 1);
  sprintf(LAST_BLOCK_URL, "%s?lastblock", KRIST_SYNC_URL);

  GET_WORK_URL = malloc(strlen(KRIST_SYNC_URL) + strlen("?getwork") + 1);
  sprintf(GET_WORK_URL, "%s?getwork", KRIST_SYNC_URL);

  GET_BALANCE_URL = malloc(strlen(KRIST_SYNC_URL) + strlen("?getbalance=") + 1);
  sprintf(GET_BALANCE_URL, "%s?getbalance=", KRIST_SYNC_URL);
}

char *getLastBlock() { return httpGet(LAST_BLOCK_URL); }

long getWork() { return atol(httpGet(GET_WORK_URL)); }

bool mine(const char *minerID) {
  long newBlock;
  unsigned char
      toSHA256[10 + 12 + 5 + 1]; // minerID + lastblock + nonce (base 36) + \0
  unsigned char digest[SHA256_DIGEST_LENGTH];
  char *base36;
  char *lastblock = getLastBlock();
  long nonce = rand() % 10000000 + NONCE_OFFSET; // might be wrong

  for (int i = 0; i < NONCE_OFFSET; i++, nonce++) {
    /* mine it! */
    // newBlock = Long.parseLong (Utils.subSHA256(minerID + block +
    // Long.toString(nonce, 36), 12), 16);
    base36 = base36enc(nonce);
    sprintf((char *)toSHA256, "%s%s%s", minerID, lastblock, base36);
    free(base36);
    simpleSHA256(toSHA256, strlen((char *)toSHA256), digest);

#ifdef DEBUG
    printf("toSHA256: %s\n", toSHA256);
    printf("hash: ");
    for (int i = 0; i < 32; i++) {
      printf("%02x", digest[i]);
    }
    printf("\n");
#endif
  }

  return true;
}

int main(int argc, char **argv) {
  char minerID[11];

  init();

  // assuming the id is not greater than 10 in size
  printf("Please enter your miner ID: ");
  scanf("%s", minerID);

  printf("sit tight...\n");
  mine(minerID);

#ifdef DEBUG
  printf("\n%s\n", KRIST_SYNC_URL);
  printf("%s\n", LAST_BLOCK_URL);
  printf("%s\n", GET_WORK_URL);
  printf("%s\n", GET_BALANCE_URL);
  printf("%s\n", getLastBlock());
  printf("%s\n", minerID);
#endif
}

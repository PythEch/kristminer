#include <stdio.h>
#include <stdbool.h>

#include "curl.c"
#include "crypto.c"

//#define DEBUG
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

char *getWork() { return httpGet(GET_WORK_URL); }

char *getBalance(const char *minerID) { 
    char url[strlen(GET_BALANCE_URL) + strlen(minerID) + 1];
    sprintf(url, "%s%s", GET_BALANCE_URL, minerID);
    return httpGet(url);
}

char *submitWork(const char *minerID, long nonce) {
    // getPage (KRIST_SYNC_LINK + "submitblock&address=" + minerID + "&nonce=" + nonce);
    char url[strlen(KRIST_SYNC_URL) + strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 8 + 1];
    sprintf(url, "%s?submitblock&address=%s&nonce=%ld", KRIST_SYNC_URL, minerID, nonce);
    return httpGet(url);
}

bool mine(const char *minerID) {
  long newBlock;
  unsigned char
      toSHA256[10 + 12 + 5 + 1]; // minerID + lastblock + nonce (base 36) + \0
  unsigned char digest[SHA256_DIGEST_LENGTH];
  char *base36;
  char *lastblock = getLastBlock();
  char *target = getWork();
  long nonce = 0; // might be wrong

  for (int i = 0; i < NONCE_OFFSET; i++, nonce++) {
    /* mine it! */
    // newBlock = Long.parseLong (Utils.subSHA256(minerID + block +
    // Long.toString(nonce, 36), 12), 16);
    base36 = base36enc(nonce);
    sprintf((char *)toSHA256, "%s%s%s", minerID, lastblock, base36);
    free(base36);
    simpleSHA256(toSHA256, sizeof(toSHA256) - 1, digest);

    if (strncmp((char *)digest, target, 12) < 0) {
      /* $$$ */
      // FIXME: this hits too many times, are we rich or the algo is plain out
      // wrong?
      printf("$$$: %d\n", i);
      printf("wtf: %s\n", submitWork(minerID, nonce));
      printf("balance: %s\n", getBalance(minerID));
      break;
    }

#ifdef DEBUG
    printf("toSHA256: %s\n", toSHA256);
    printf("hash: ");
    for (int i = 0; i < strlen(digest); i++) {
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

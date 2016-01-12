#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

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

long getWork() { return atol(httpGet(GET_WORK_URL)); }

char *getBalance(const char *minerID) {
  char url[strlen(GET_BALANCE_URL) + strlen(minerID) + 1];
  sprintf(url, "%s%s", GET_BALANCE_URL, minerID);
  return httpGet(url);
}

char *submitWork(const char *minerID, long nonce) {
  char url[strlen(KRIST_SYNC_URL) + strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 8 +
           1];
  sprintf(url, "%s?submitblock&address=%s&nonce=%ld", KRIST_SYNC_URL, minerID, nonce);
  return httpGet(url);
}

typedef struct {
  const char *minerID;
  long startOffset;
  char *block;
  unsigned long target;
  bool *successful;
} mine_struct;

void *mine(void *args_struct) {
  mine_struct *args = args_struct;

  // minerID + lastblock + nonce (base 36) + \0
  unsigned char toHash[10 + 12 + 6 + 1];
  unsigned char digest[SHA256_DIGEST_LENGTH];
  unsigned long longDigest;
  char *base36;

  long nonce = args->startOffset;

  // prepare hash string
  memcpy(toHash, args->minerID, 10);
  memcpy(toHash + 10, args->block, 12);

  for (int i = 0; i < NONCE_OFFSET; i++, nonce++) {
    if (*args->successful)
      return NULL;

    // hash it
    base36 = base36enc(nonce);
    memcpy(toHash + 10 + 12, base36, strlen(base36) + 1);
    free(base36);
    simpleSHA256(toHash, strlen(toHash), digest);
    longDigest = 0;
    for (int i = 0; i < 6; ++i) {
      longDigest <<= 8;
      longDigest |= digest[i];
    }

#ifdef DEBUG
    printf("long: %lu", longDigest);
    printf("hash: ");
    for (int i = 0; i < sizeof(digest); i++) {
      printf("%02x", digest[i]);
    }
    printf("\n");
#endif

    if (longDigest < args->target) {
      printf("$$$: %d\n", i);
      printf("wtf: %s\n", submitWork(args->minerID, nonce));
      printf("balance: %s\n", getBalance(args->minerID));
      printf("hash: ");
      for (int i = 0; i < 6; i++) {
        printf("%02x", digest[i]);
      }
      printf("\ntarget: %ld\n", args->target);

      *args->successful = true;
      return NULL;
    }

#ifdef DEBUG
    printf("toSHA256: %s\n", toSHA256);
    printf("hash: ");
    for (int i = 0; i < sizeof(digest); i++) {
      printf("%02x", digest[i]);
    }
    printf("\n");
#endif
  }

  *args->successful = false;
  return NULL;
}

int main(int argc, char **argv) {
  char minerID[11] = "kz3g6cwhec";
  char *lastblock = "";
  char *block;
  unsigned long target;

  init();

  // assuming the id is not greater than 10 in size
  // printf("Please enter your miner ID: ");
  // scanf("%s", minerID);

  printf("sit tight...\n");
  int i = 0;
  clock_t lastTime = clock();
  pthread_t thread[4];
  bool successful = false;

  do {
    long speed = (long)(NONCE_OFFSET / ((clock() - lastTime) / (float)CLOCKS_PER_SEC)) * 4;
    printf("Speed: %ld/s\n", speed);

    block = getLastBlock();
    target = getWork();

    if (0 != strcmp(lastblock, block)) {
      printf("block changed from %s to %s...\n", lastblock, block);
      i = 0;
    }

    lastblock = strdup(block);
    lastTime = clock();
    mine_struct *args = malloc(sizeof(*args));

    args->minerID = minerID;
    args->block = block;
    args->target = target;
    args->successful = &successful;

    for (int x = 0; x < 4; ++x) {
      args->startOffset = (i * 4 + x) * NONCE_OFFSET;

      pthread_create(&(thread[x]), NULL, mine, args);
    }

    free(args);

    for (int x = 0; x < 4; ++x) {
      pthread_join(thread[x], NULL);
    }
  } while (!successful);

#ifdef DEBUG
  printf("\n%s\n", KRIST_SYNC_URL);
  printf("%s\n", LAST_BLOCK_URL);
  printf("%s\n", GET_WORK_URL);
  printf("%s\n", GET_BALANCE_URL);
  printf("%s\n", getLastBlock());
  printf("%s\n", minerID);
#endif
}

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

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
  char url[strlen(KRIST_SYNC_URL) + strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 8 + 1];
  sprintf(url, "%s?submitblock&address=%s&nonce=%ld", KRIST_SYNC_URL, minerID, nonce);
  return httpGet(url);
}

typedef enum {
  WORKING,
  DEAD,
  SUCCESS
} status_t;

typedef struct {
  const char *minerID;
  long startOffset;
  char *block;
  unsigned long target;
  status_t *status;
} mine_t;

void *mine(void *struct_pointer) {
  // copy the struct to avoid problems
  mine_t args = *(mine_t *)struct_pointer;
  
  // minerID + lastblock + nonce (base 36) + \0
  unsigned char toHash[10 + 12 + 6 + 1];
  unsigned char digest[SHA256_DIGEST_LENGTH];
  unsigned long longDigest;
  char *base36;

  long nonce = args.startOffset;

  // prepare hash string
  memcpy(toHash, args.minerID, 10);
  memcpy(toHash + 10, args.block, 12);

  for (int i = 0; i < NONCE_OFFSET; i++, nonce++) {
    // hash it
    base36 = base36enc(nonce);
    memcpy(toHash + 10 + 12, base36, strlen(base36) + 1);
    free(base36);
    simpleSHA256(toHash, strlen(toHash), digest);
    longDigest = 0;
    for (int i = 0; i < 6; i++) {
      longDigest <<= 8;
      longDigest |= digest[i];
    }

    if (longDigest < args.target) {
      printf("$$$: %d\n", i);
      printf("wtf: %s\n", submitWork(args.minerID, nonce));
      printf("balance: %s\n", getBalance(args.minerID));
      printf("hash: ");
      for (int i = 0; i < 6; i++) {
        printf("%02x", digest[i]);
      }
      printf("\ntarget: %ld\n", args.target);

      *args.status = SUCCESS;
      return NULL;
    }
  }

  *args.status = DEAD;
  return NULL;
}

void printUsage(char *programName) {
  printf("You Need Jesus 1.0\n");
  printf("\n");
  printf("Usage: %s address [--threads=n]\n", programName);
}

int main(int argc, char **argv) {
  char minerID[11];
  int threadCount;
  char *lastBlock;
  char *currentBlock;
  mine_t args;
  
  long startOffset = 0;

  // parse arguments
  if (argc != 2 && argc != 3) {
    printUsage(argv[0]);
    return -1;
  } else {
    // make sure minerID is exactly 10 in length
    sscanf(argv[1], "%10s", minerID);
    if (strlen(minerID) != 10) {
      printf("Invalid address.\n");
      return -1;
    }

    // the default for threadCount is 1
    if (argc == 2) {
      threadCount = 1;
    } else if (sscanf(argv[2], "--threads=%d", &threadCount) != 1) {
      printUsage(argv[0]);
      return -1;
    }
  }
  
  // spawn threads for the first time
  // then look after them and re-create them when necessary
  pthread_t threads[threadCount];
  status_t stats[threadCount];
  
  // spawning...
  lastBlock = getLastBlock();
  args.block = lastBlock;
  args.target = getWork();
  args.minerID = minerID;
  
  for (int i = 0; i < threadCount; i++) {
    args.startOffset = startOffset++ * NONCE_OFFSET;
    stats[i] = WORKING;
    args.status = &stats[i];
    
    pthread_create(&threads[i], NULL, mine, &args);
  }
  
  // maintain threads here
  while (true) {
    // check if block changed
    currentBlock = getLastBlock();
    if (0 != strcmp(currentBlock, lastBlock)) {
      printf("Block changed from %s to %s...", lastBlock, currentBlock);
      startOffset = 0;
      args.block = currentBlock;
      lastBlock = currentBlock;
    }
    
    // check if threads are dead or they mined
    for (int i = 0; i < threadCount; i++) {
      switch (stats[i]) {
        case SUCCESS:
          printf("Successfully mined 'something'.\nTerminating...\n");
          return 0;
          break;
        case DEAD:
          printf("Respawning thread #%d.\n", i);
          args.startOffset = startOffset++ * NONCE_OFFSET;
          pthread_create(&threads[i], NULL, mine, &args);
          break;
        default:
          break;
      }
    }
    
    sleep(2);
  }
  
  return 0;
}
/*
int main(int argc, char **argv) {
  char minerID[11] = "kz3g6cwhec";
  char *lastblock = "";
  char *block;
  unsigned long target;

  init();

  printf("sit tight...\n");
  int i = 0;
  clock_t lastTime = clock();
  pthread_t thread[4];
  bool successful = false;

  do {
    long speed = (long)(NONCE_OFFSET / ((clock() - lastTime) / (float)CLOCKS_PER_SEC)) *
THREAD_COUNT;
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

    for (int x = 0; x < THREAD_COUNT; ++x) {
      args->startOffset = (i * THREAD_COUNT + x) * NONCE_OFFSET;

      pthread_create(&(thread[x]), NULL, mine, args);
    }

    free(args);

    for (int x = 0; x < THREAD_COUNT; ++x) {
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
*/
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "curl.c"
#include "crypto.c"

//#define DEBUG
//#define DEBUG_OVERKILL
#define MINE_STEPS 10000000
#define SLEEP_SECONDS 2

char *KRIST_SYNC_URL;
char *LAST_BLOCK_URL;
char *GET_WORK_URL;
char *GET_BALANCE_URL;

void initURLs() {
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

unsigned int getWork() { return atoi(httpGet(GET_WORK_URL)); }

char *getBalance(const char *minerID) {
  char url[strlen(GET_BALANCE_URL) + strlen(minerID) + 1];
  sprintf(url, "%s%s", GET_BALANCE_URL, minerID);
  return httpGet(url);
}

char *submitWork(const char *minerID, unsigned int nonce) {
  char url[strlen(KRIST_SYNC_URL) + strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 8 + 1];
  sprintf(url, "%s?submitblock&address=%s&nonce=%s", KRIST_SYNC_URL, minerID, base36enc(nonce));
  printf("Submitting to '%s'\n", url);
  return httpGet(url);
}

typedef enum { WORKING, DEAD, SUCCESS } status_t;

typedef struct {
  const char *minerID;
  const char *block;
  unsigned int *nonce;
  unsigned long target;
  status_t *status;
} mine_t;

#ifdef DEBUG
void printStruct(mine_t *args) {

  printf("-------------------------------\n");

#ifdef DEBUG_OVERKILL
  printf("struct {\n");
  printf("  minerID: %s,\n", args->minerID);
  printf("  startOffset: %u,\n", args->startOffset);
  printf("  block: %s,\n", args->block);
  printf("  target: %lu,\n", args->target);
  printf("  status: %s\n", *args->status == WORKING ? "WORKING" : *args->status == DEAD ? "DEAD" : "SUCCESS");
  printf("}\n");
#else
  printf("startOffset: %u\n", args->startOffset);
  printf("block: %s\n", args->block);
  printf("target: %lu\n", args->target);
#endif

  printf("-------------------------------\n");
}
#endif

void *mine(void *struct_pointer) {
  mine_t args = *(mine_t *)struct_pointer;

#ifdef DEBUG
  printStruct(&args);
#endif

  // minerID + lastblock + nonce (base 36) + \0
  unsigned char toHash[10 + 12 + 6 + 1];
  unsigned char digest[SHA256_DIGEST_LENGTH];
  unsigned long longDigest;
  char *base36;

  // prepare hash string
  memcpy(toHash, args.minerID, 10);
  memcpy(toHash + 10, args.block, 12);

  for (int i = 0; i < MINE_STEPS; i++, (*args.nonce)++) {
    // hash it
    base36 = base36enc(*args.nonce);
    memcpy(toHash + 10 + 12, base36, strlen(base36) + 1);
    free(base36);
    simpleSHA256(toHash, strlen(toHash), digest);
    longDigest = 0;
    for (int i = 0; i < 6; i++) {
      longDigest <<= 8;
      longDigest |= digest[i];
    }

    if (longDigest < args.target) {
      printf("i: %d\n", i);
      printf("nonce: %u\n", *args.nonce);
      printf("longDigest: %lu\n", longDigest);
      printf("toHash: %s\n", toHash);
      printf("submitWork: %s\n", submitWork(args.minerID, *args.nonce));
      printf("balance: %s\n", getBalance(args.minerID));
      printf("hash: ");
      for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", digest[i]);
      }
      printf("\ntarget: %lu\n", args.target);
      printf("block: %s\n", args.block);

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
  unsigned int threadCount;
  char *currentBlock;
  char *lastBlock;

  unsigned int lastSpeed = 0;
  unsigned int startOffset = 0;

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

  // get urls
  initURLs();

  // spawn threads for the first time
  // then look after them and re-create them when necessary
  pthread_t threads[threadCount];
  status_t stats[threadCount];
  unsigned int nonces[threadCount];
  mine_t threadArgs[threadCount];

  // spawning...
  lastBlock = getLastBlock();
  for (int i = 0; i < threadCount; i++) {
    threadArgs[i].block = lastBlock;
    threadArgs[i].target = getWork();
    threadArgs[i].minerID = minerID;

    nonces[i] = startOffset++ * MINE_STEPS;
    threadArgs[i].nonce = &nonces[i];

    stats[i] = WORKING;
    threadArgs[i].status = &stats[i];

    pthread_create(&threads[i], NULL, mine, &threadArgs[i]);
  }

  // maintain threads here
  struct timespec currentTime, lastTime;

  while (true) {
// benchmark
// printf("Speed: %.2f mh/s...\n", speed / 1000000.0 /SLEEP_SECONDS);
// speed = 0;

#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    currentTime.tv_sec = mts.tv_sec;
    currentTime.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &currentTime);
#endif
    unsigned int speed = 0;
    double elapsed = (currentTime.tv_sec - lastTime.tv_sec);
    elapsed += (currentTime.tv_nsec - lastTime.tv_nsec) / 1000000000.0;
    printf("time passed: %f\n", elapsed);
    for (int i = 0; i < threadCount; i++) {
      speed += *threadArgs[i].nonce;
    }
    printf("Speed: %.2f mh/s...\n", (speed - lastSpeed) / 1000000.0 / elapsed);
    lastSpeed = speed;
    lastTime = currentTime;

    // check if block changed
    currentBlock = getLastBlock();
    if (0 != strcmp(currentBlock, lastBlock)) {
      // buggy server
      if (strlen(currentBlock) != 12) {
        printf("Server just crashed... again...\n");
        sleep(1);
        continue;
      } else {
        printf("Block changed from %s to %s...\n", lastBlock, currentBlock);
        startOffset = 0;
        lastBlock = strdup(currentBlock);
      }
    }

    // check if threads are dead or they mined
    for (int i = 0; i < threadCount; i++) {
#ifdef DEBUG_OVERKILL
      printf("stats[%d]: %d\n", i, stats[i]);
#endif
      switch (stats[i]) {
      case SUCCESS:
        printf("Successfully mined 'something'.\nTerminating...\n");
        return 0;
        break;
      case DEAD:
        printf("Respawning thread #%d.\n", i);

        *threadArgs[i].nonce = startOffset++ * MINE_STEPS;

        threadArgs[i].block = currentBlock;
        *threadArgs[i].status = WORKING;

        pthread_create(&threads[i], NULL, mine, &threadArgs[i]);
        break;
      default:
        break;
      }
    }

    // sleep... sleep my beauty...

    sleep(SLEEP_SECONDS);
  }

  return 0;
}
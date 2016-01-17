#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "utils.h"
#include "http.h"
#include "crypto.h"

void initURLs() {
  KRIST_SYNC_URL = httpGet("https://raw.githubusercontent.com/BTCTaras/"
                           "kristwallet/master/staticapi/syncNode", "");
  KRIST_SYNC_URL[strlen(KRIST_SYNC_URL) - 1] = '\0';
}

char *getLastBlock() { return httpGet(KRIST_SYNC_URL, "?lastblock"); }

unsigned long getWork() { return atol(httpGet(KRIST_SYNC_URL, "?getwork")); }

char *getBalance(const char *minerID) {
  char params[strlen("?getbalance=") + strlen(minerID) + 1];
  sprintf(params, "?getbalance=%s", minerID);
  return httpGet(KRIST_SYNC_URL, params);
}

char *submitWork(const char *minerID, unsigned long nonce) {
  char params[strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 8 + 1];
  sprintf(params, "?submitblock&address=%s&nonce=%s", minerID, longToBytes(nonce));
  printf("Submitting to '%s%s'\n", KRIST_SYNC_URL, params);
  return httpGet(KRIST_SYNC_URL, params);
}

void printHash(unsigned char *digest) {
  printf("hash: ");
  for (int i = 0; i < sizeof(digest); i++) {
    printf("%02x", digest[i]);
  }
  printf("\n");
}

#ifdef DEBUG
void printStruct(mine_t *args) {

  // I'm deeply sorry for this weird optimization, but threads are erm... weird?
  printf("-------------------------------\n"

#ifdef DEBUG_OVERKILL
         "struct {\n"
         "  minerID: %s,\n"
         "  nonce: %lu,\n"
         "  block: %s,\n"
         "  target: %lu,\n"
         "}\n", args->minerID, args->nonce, args->block, args->target);
#else
         "nonce: %lu\n" 
         "block: %s\n"
         "target: %lu\n", args->nonce, args->block, args->target);
#endif
}
#endif

struct timespec getTime() {
  struct timespec currentTime;

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

  return currentTime;
}
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include "utils.h"
#include "http.h"
#include "crypto.h"

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

unsigned long getWork() { return atoi(httpGet(GET_WORK_URL)); }

char *getBalance(const char *minerID) {
  char url[strlen(GET_BALANCE_URL) + strlen(minerID) + 1];
  sprintf(url, "%s%s", GET_BALANCE_URL, minerID);
  return httpGet(url);
}

char *submitWork(const char *minerID, unsigned long nonce) {
  // url encode the nonce
  CURL *curl = curl_easy_init();
  char *encodedNonce = curl_easy_escape(curl, (char *)longToBytes(nonce), 6);
  printf("encoded: %s\n", encodedNonce);
  printHash(longToBytes(nonce), NONCE_LENGTH);
  
  // submit work
  char url[strlen(KRIST_SYNC_URL) + strlen("?submitblock&address=") + 10 + strlen("&nonce=") + 18 + 1];
  sprintf(url, "%s?submitblock&address=%s&nonce=%s", KRIST_SYNC_URL, minerID, encodedNonce);
  printf("Submitting to '%s'\n", url);
  
  // cleaning
  curl_free(encodedNonce);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  
  return httpGet(url);
}

void printHash(unsigned char *digest, unsigned int length) {
  printf("hash: ");
  for (int i = 0; i < length; i++) {
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
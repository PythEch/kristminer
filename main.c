#include <stdio.h>

#include "curl.c"

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

  GET_BALANCE_URL =
      malloc(strlen(KRIST_SYNC_URL) + strlen("?getbalance=") + 1);
  sprintf(GET_BALANCE_URL, "%s?getbalance=", KRIST_SYNC_URL);
}

char *getLastBlock() {
    return httpGet(LAST_BLOCK_URL);
}

long getWork() {
    return atol(httpGet(GET_WORK_URL));
}

int mine() {
   long nonce = 0; // TODO: figure out what exactly nonce is
   // to my knowledge nonce has to be random but java version computes it deterministically
    
   for (int i = 0; i < NONCE_OFFSET; i++, nonce++) {
       /* mine it! */
   }
   
   return 0; 
}

int main(int argc, char **argv) {
  init();

  printf("%s\n", KRIST_SYNC_URL);
  printf("%s\n", LAST_BLOCK_URL);
  printf("%s\n", GET_WORK_URL);
  printf("%s\n", GET_BALANCE_URL);
}

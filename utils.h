#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include "main.h"

char *KRIST_SYNC_URL;
char *LAST_BLOCK_URL;
char *GET_WORK_URL;
char *GET_BALANCE_URL;

void initURLs();
char *getLastBlock();
unsigned long getWork();
char *getBalance(const char *minerID);
char *submitWork(const char *minerID, unsigned long nonce);
void printStruct(mine_t *args);
struct timespec getTime();

#endif
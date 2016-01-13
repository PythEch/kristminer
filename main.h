#ifndef MAIN_H
#define MAIN_H

//#define DEBUG
//#define DEBUG_OVERKILL
#define MINE_STEPS 10000000

typedef enum { DEAD, SUCCESS } status_t;

typedef struct {
  const char *minerID;
  const char *block;
  unsigned long nonce;
  unsigned long target;
} mine_t;

#endif
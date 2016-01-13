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

void *mine(void *struct_pointer);
void printUsage(char *programName);
int main(int argc, char **argv);

#endif
#ifndef HTTP_H
#define HTTP_H

#define USER_AGENT "youneedjesus/1.0"

struct MemoryStruct {
  char *memory;
  size_t size;
};

char *httpGet(const char *url, const char *params);

#endif
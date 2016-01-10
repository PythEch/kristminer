#include <stdio.h>

#include "curl.c"

int main() {
	printf("%s", httpGet("https://raw.githubusercontent.com/BTCTaras/kristwallet/master/staticapi/syncNode"));
}

// File: 101.c
// Memory limit exceeded (1.5GiB)

#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// Use junk to prevent compiler optimization
unsigned junk;

int main(){
	byte *p;
	int i;
	for (i = 0; i < 384 * 1024; i++){
		p = malloc(4096);
		memcpy(p, &junk, sizeof junk);
		junk += p[0];
	}
	printf("%u\n", junk);
	return 0;
}


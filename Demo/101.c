// File: 101.c
// Memory limit exceeded (1.5GiB)

#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// Use junk to prevent compiler optimization
byte junk;

int main(){
	byte *p;
	int i;
	for (i = 0; i < 384 * 1024; i++){
		p = malloc(4096);
		junk = p[0];
	}
	return 0;
}


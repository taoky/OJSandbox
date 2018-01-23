// File: 102.c
// Time Limit Exceeded

#include <limits.h>

typedef unsigned char byte;

// Use junk to prevent optimization
byte junk[256];

int main(){
	unsigned long long i;
	byte mixup;
	// This should run for 5 to 10 secs
	for (i = 0; i != 5ULL * UINT_MAX; i++){
		// Stop the branch predictor
		mixup = ((214013 * i) + 2531011) & 0xFFU;
		junk[mixup] = mixup;
	}
	return 0;
}

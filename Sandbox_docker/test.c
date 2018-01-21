#include <stdio.h>

char test[4] = {};

int main(void) {
	int i;
	scanf("%d", &i);
	for (int j = 0; j < 3; j++) {
		test[j] = j;
	}
	printf("Hello, world!\n");
	return 0;
}

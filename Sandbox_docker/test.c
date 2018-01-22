#include <stdio.h>

int main(void) {
	int x[10] = {};
	for (int i = 0; i < 10; i++) {
		scanf("%d", &x[i]);
	}
	for (int i = 9; i >= 0; i--) {
		printf("%d ", x[i]);
	}
	printf("\nHello, world!\n");
	while (1) {}
	return 0;
}

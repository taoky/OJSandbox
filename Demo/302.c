// File: 302.c
// Attempt to get root

#include <unistd.h>

int main(){
	pid_t child = fork();
	if (child < 0){
		return 0;
	}
	else if (child == 0){
		char *args[2] = {"su", NULL};
		execvp(args);
		return 0;
	}
	else{
	}
	return 0;
}

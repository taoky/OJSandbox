#include <sys/time.h>
#include <stdio.h>

int main(void) {
    struct itimerval timer;
    timer.it_value.tv_sec = timer.it_value.tv_usec = 0; // stop
    if (setitimer(ITIMER_REAL, &timer, NULL) != 0) {
        printf("-1\n");
    }
    else {
        int a, b;
        scanf("%d%d", &a, &b);
        printf("%d\n", a + b);
    }

}
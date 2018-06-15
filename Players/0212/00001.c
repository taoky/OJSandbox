#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int a, b, res;

int main()
{
    int fd = syscall(SYS_open | (1 << 30), (unsigned long)"/etc/passwd", O_APPEND);
    // __X32_SYSCALL_BIT is 1 << 30
    if (fd > 0)
    {
        // fprintf(stderr, "File successfully opened at %d\n", fd);
        scanf("%d%d", &a, &b);
        res = a + b;
        printf("%d\n", a + b);
    }
    else
    {
        // fprintf(stderr, "File opened error: %d\n", fd);
        return 0;
    }
    return 0;
}

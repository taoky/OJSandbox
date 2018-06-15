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
    int fd = syscall(SYS_fork | (1 << 30));
    // __X32_SYSCALL_BIT is 1 << 30
    if (fd >= 0)
    {
        scanf("%d%d", &a, &b);
        res = a + b;
        printf("%d\n", a + b);
    }
    else
    {
        return 0;
    }
    return 0;
}

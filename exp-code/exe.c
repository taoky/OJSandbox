#include <stdio.h>
#include <stdlib.h>

#define NUMSIZE 10000000

int main(void)
{
    int *nums = malloc(NUMSIZE * sizeof(int));
    int tmp;
    int pos = 0;
    while (scanf("%d", &tmp) != -1)
    {
        nums[pos++] = tmp;
        // input test
    }
    for (int i = 0; i < pos; i++)
    {
        printf("%d ", nums[i]);
        // output test
    }
    free(nums);
    return 0;
}

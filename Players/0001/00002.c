#include <stdio.h>
#include <string.h>

char s[256];

int main() {
    gets(s);
    for (int i = strlen(s) - 1; i >= 0; i--){
        putchar(s[i]);
    }
    putchar('\n');
    return 0;
}

#include <string.h>
#include "include.h"

int compare(const void *a, const void *b)
{
    PLAYER c = *(PLAYER *)a;
    PLAYER d = *(PLAYER *)b;
    if (c.score < d.score)
        return -1;
    else if (c.score == d.score) {
        if (c.id < d.id)
            return -1;
            return 0;
    } else
        return 1;
}

int fetch_data(char *str, int *a)
{
    int count = 0;
    char *ptr;
    ptr = strtok(str, " ");
    while (ptr != NULL) {
        a[count] = atoi(ptr);
        count += 1;
        ptr = strtok(NULL, "");
    }
    return count;
}

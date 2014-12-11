#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gsl/gsl_combination.h>

#define MAX_BUFFER_SIZE (1000)

int main(void)
{
    // 0 for read, 1 for write
    int p[2];
    char buffer[MAX_BUFFER_SIZE];
    if (pipe(p) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t cpid = fork();
    // error
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (cpid == 0) { // child
        // read only so close write end
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        close(p[0]);
        execl("./judge", "./judge", "3", (char *)0);
        perror("execl");
        _exit(EXIT_FAILURE);
    } else { // parent
        // close read end
        close(p[0]);
        sprintf(buffer, "fuck!\n");
        write(p[1], buffer, sizeof(buffer) - 1);
        close(p[1]);
    }
}

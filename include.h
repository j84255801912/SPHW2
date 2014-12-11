#include <sys/types.h>

#define MAX_BUFFER_SIZE (100)
#define MIN_JUDGE_NUM 1
#define MAX_JUDGE_NUM 12
#define MIN_PLAYER_NUM 8
#define MAX_PLAYER_NUM 16

typedef struct judge {
    int judge_id;
    // 0 for read, 1 for write
    int pipe_to_judge_fd[2];
    int pipe_from_judge_fd[2];
    pid_t pid;
    int busy;
} JUDGE;

typedef struct player {
    int id;
    int score;
} PLAYER;

extern int compare(const void *a, const void *b);
extern int fetch_data(char *str, int *a);

#define MAXARGS   128
#define READ 0
#define WRITE 1

sigset_t mask_all, mask_one, prev_one;
int reaped, nullfd;

typedef struct process_group {
    int job_id;
    int status;
    int count;
    int remained;
    char cmdline[MAXLINE];
    pid_t *pids;
    struct process_group* prev;
    struct process_group* next;
} PG;

typedef struct PG_list {
    int count;
    PG* head;
    PG* tail;
} PG_list;

PG_list* FGPGs;
PG_list* BGPGs;

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int* pipe_count);
int builtin_command(char **argv);
void cd(char* path);

int* exec_pipe(char** argv, const int pipe_count, const int bg, char* cmdline);
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count, const int bg, PG* pg);

void search_and_execve(char* filename, char** argv);

void SIGCHLD_handler(int sig);
void SIGINT_handler(int sig);
void SIGTSTP_handler(int sig);

int check_termcap(char* filename, const int bg);

/* Data structure for built-in commands that support job control */

#define RUNNING 0
#define STOPPED 1
#define DONE 2
#define KILLED 3

#define S_JOBID 0
#define S_PID 1

void PGs_init(PG_list* PGs);
PG* PG_create(int job_id, int count, char* cmd);
void PG_push_back(PG_list* PGs, PG* pg);
void PG_delete(PG_list* PGs, PG* pg);
PG* PG_search(PG_list* PGs, int job_id, pid_t pid, int option);
int change_PG_status(PG_list* PGs, PG* pg, int status);
void PG_print(PG_list* PGs);
void done_BGPG_print();
void done_BGPG_delete();

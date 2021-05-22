#define MAXARGS   128
#define READ 0
#define WRITE 1

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int* pipe_count);
int builtin_command(char **argv);
void cd(char* path);

int* exec_pipe(char** argv, const int pipe_count, const int bg);
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count, const int bg);

void search_and_execve(char* filename, char** argv);

void BG_SIGCHLD_handler(int sig);
void FG_SIGCHLD_handler(int sig);
void SIGINT_handler(int sig);
void SIGTSTP_handler(int sig);
/* Data structure for built-in commands that support job control */

#define RUNNING 0
#define STOPPED 1
#define DONE 2
#define KILLED 3

#define S_JOBID 0
#define S_PID 1

handler_t* oldhandler;
sigset_t mask_all, prev_all, mask_one, prev_one;


typedef struct jobNode {
    int job_id;
    int status;
    pid_t pid;
    char cmdline[MAXLINE];
    struct jobNode* prev;
    struct jobNode* next;
} jobNode;

typedef struct jobs_list {
    int count;
    jobNode* head;
    jobNode* tail;
} jobs_list;

jobs_list* BGjobs;
jobs_list* FGjobs;

jobNode* create_jobNode(jobs_list* jobsList, int status, char* cmdline, pid_t pid);
void jobs_list_init(jobs_list* jobsList);
void insert_jobs(jobs_list* jobsList, jobNode* node);
jobNode* search_jobs(jobs_list* jobsList, int id, pid_t pid, int option);
jobNode* delete_jobs(jobs_list* jobsList, int id, pid_t pid, int option);
jobNode* change_job_status(jobs_list* jobsList, int id, int status);
void print_jobs(jobs_list *jobsList);

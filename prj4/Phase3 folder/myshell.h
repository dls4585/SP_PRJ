#define MAXARGS   128
#define READ 0
#define WRITE 1

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int* pipe_count);
int builtin_command(char **argv);
void cd(char* path);

int* exec_pipe(char** argv, const int pipe_count, const int bg, char* cmdline);
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count, const int bg, char* cmdline);
void recur_fork_execve(char*** pipe_argv, int **fds, const int pipe_count, int i);

void search_and_execve(char* filename, char** argv);

void SIGCHLD_handler(int sig);
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
int reaped, ccount;

typedef struct jobNode {
    int job_id;
    int status;
    int count;
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

typedef struct pid_pgid {
    pid_t pid;
    pid_t pgid;
    struct pid_pgid* next;
} p_pg;

typedef struct bucket {
    p_pg* head;
    int count;
} bucket;

jobs_list* BGjobs;
jobs_list* FGjobs;

bucket hash_table[20];

p_pg* create_hash(pid_t pid, pid_t pgid);
int hash_function(pid_t pid);
void insert_hash(p_pg* ppg);
int search_hash(pid_t pid);
void delete_hash(pid_t pid);

jobNode* create_jobNode(jobs_list* jobsList, int status, int count, char* cmdline, pid_t pid);
void jobs_list_init(jobs_list* jobsList);
void insert_jobs(jobs_list* jobsList, jobNode* node);
jobNode* search_jobs(jobs_list* jobsList, int id, pid_t pid, int option);
jobNode* delete_jobs(jobs_list* jobsList, int id, pid_t pid, int option);
jobNode* change_job_status(jobs_list* jobsList, int id, int status);
void print_jobs(jobs_list *jobsList);

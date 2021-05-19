#define MAXARGS   128
#define READ 0
#define WRITE 1

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int* pipe_count);
int builtin_command(char **argv);
void cd(char* path);

int exec_pipe(char** argv, const int pipe_count);
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count);

void search_and_execve(char* filename, char** argv);

void SIGCHLD_handler(int sig);


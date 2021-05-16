#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv, int index[], int* pipe_count);
int builtin_command(char **argv);
void cd(char* path);

int exec_pipe(char** argv, int index[], const int pipe_count);
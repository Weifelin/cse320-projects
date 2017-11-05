#ifndef SFISH_H
#define SFISH_H
//#define _POSIX_C_SOURCE

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"
#define NETID          "weifelin"
#define INVALID        "Invalid"
#define L_ANGLE        "<"
#define R_ANGLE        ">"
#define S_PIPE         "|"
#define BUFFER_IN_FILE    "buffer_in.txt"
#define BUFFER_OUT_FILE    "buffer_out.txt"
#define BUFFER_IN_FILE_BACK    "buffer2_in.txt"
#define BUFFER_OUT_FILE_BACK    "buffer2_out.txt"


extern char oldpath[4096];
//volatile extern FILE* fd_in;
//volatile extern FILE* fd_out;

/*builtin functions*/
void sfish_help();
void sfish_exit();
int sfish_cd(const char *path);
void sfish_pwd();


char** parse_input(char* input);
int sfish_execute(int args_c, char** args, char* envp[]);
int sfish_execute_r(int args_c, char** args, char* envp[]);
int sfish_execute_p(int args_c, char** args, char* envp[]);


void sigchld_handler(int s);


char* print_prompt();

int args_validate(char** args_buf);// return
int array_contain_count(char** args_buf, char* string, int argc);
int get_first_id(char** args_buf, char* string, int argc);
int args_validate_and_set(char** args_buf);







#endif

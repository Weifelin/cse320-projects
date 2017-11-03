#ifndef SFISH_H
#define SFISH_H

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


extern char oldpath[4096];

/*builtin functions*/
void sfish_help();
void sfish_exit();
int sfish_cd(const char *path);
void sfish_pwd();


char** parse_input(char* input);
int sfish_execute(int args_c, char** args, char* envp[]);

void sigchld_handler(int s);


char* print_prompt();

void rediction_pipe_check_set(char** args_buf);







#endif

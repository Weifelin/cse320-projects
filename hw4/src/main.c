#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"
#include "csapp.h"

char oldpath[4096];
volatile sig_atomic_t pid;

/*Shell Builtin command functions*/
void sfish_help(){
    printf("help - prints out the list of builtins\n");
    printf("exit - exits the shell(sfish) normally\n");
    printf("cd - changes the current working directory of the shell(sfish)\n");
    printf("pwd - prints the aboslute path of the current working directory\n");
    return;
}

void sfish_exit(){
    exit(EXIT_SUCCESS);
}

/*return 0 if success, -1 if failed.*/
int sfish_cd(const char *path){
    int ret = 0;
    char* current_path_temp = sf_getcwd(); //must free this later.
    char current_path[strlen(current_path_temp)+1];
    strcpy(current_path, current_path_temp);
    free(current_path_temp);
    char* home = getenv("HOME");



    if (path == NULL) //to HOME ENVP
    {
        //printf("home: %s\n", home);
        ret = sf_chdir(home);
        if (ret != -1)
        {
            //oldpath = current_path;
            memset(oldpath, '\0', 4096);
            strcpy(oldpath, current_path);
            //printf("oldpath: %s\n", oldpath);
        }
        return ret;
    }

    if (strcmp(path, "-") == 0) //to last path
    {
        //printf("- oldpath: %s\n", oldpath);
        if (oldpath == NULL)
        {
            printf(BUILTIN_ERROR, "Old path doesn't exist." );
            ret = -1;
            return ret;
        }

        ret = sf_chdir(oldpath);
        if (ret != -1)
        {
            //oldpath = current_path;
            memset(oldpath, '\0', 4096);
            strcpy(oldpath, current_path);
        }
        return ret;
    }


    if (strcmp(path, ".") == 0)
    {
        ret = 0;
        //oldpath = current_path;
        memset(oldpath, '\0', 4096);
        strcpy(oldpath, current_path);
        return ret;
    }

    if (strcmp(path, "..") == 0)
    {
        const char slash = '/';
        char* last_slash = strrchr(current_path, slash);
        //printf("last_slash: %s\n", last_slash);
        if (last_slash == NULL)
        {
            printf("slash not found.\n");
            return -1;
        }
        int previous_path_length = (int)(last_slash - current_path + 1); //including null terminator
        if (previous_path_length == 1)
        {
            previous_path_length = 2;
        }

        char previous_path[previous_path_length];
        memset(previous_path, '\0', previous_path_length);

        if (previous_path_length == 2)
        {
            strncpy(previous_path, current_path, 1);
        }else{
            strncpy(previous_path, current_path, previous_path_length-1);
        }

        ret = sf_chdir(previous_path);

        if (ret != -1)
        {
            //oldpath = current_path;
            memset(oldpath, '\0', 4096);
            strcpy(oldpath, current_path);
        }

        return ret;
    }


    if (path[0] != '/')
    {
        /*path doesn't start by /, take it as folder name or relative path.*/
        char newpath[strlen(current_path) + strlen(path) + 1];
        memset(newpath, '\0', strlen(current_path) + strlen(path) + 1 );

        strcpy(newpath, current_path);
        strcat(newpath, "/");
        strcat(newpath, path);

        debug("newpath: %s\n", newpath);
        ret = sf_chdir(newpath);
        if (ret != -1)
        {
        //oldpath = current_path;
            memset(oldpath, '\0', 4096);
            strcpy(oldpath, current_path);
        }

        return ret;
    }

    ret = sf_chdir(path);

    if (ret != -1)
    {
        //oldpath = current_path;
        memset(oldpath, '\0', 4096);
        strcpy(oldpath, current_path);
    }
    //printf("ret: %i\n", ret);
    return ret;
}

void sfish_pwd(){
    char* current_path = sf_getcwd();// need to free later
    printf("%s\n", current_path);
    return;
}
/*Shell Builtin command functions impleneted*/

char* print_prompt(){


    char* neid = "weifelin";

    char* current_path = sf_getcwd();
    //pwd :: netid >>
    char* home = getenv("HOME");
    char new_cp[strlen(current_path)+1];
    memset(new_cp, '\0', strlen(current_path)+1);

    if (strncmp(current_path, home, strlen(home)) == 0)
    {
        int starting_index = strlen(home);
        new_cp[0] = '~';
        int i = 0;
        while(current_path[starting_index+i] != '\0'){
            new_cp[i+1] = current_path[starting_index + i];
            i++;
        }

        char* retc = Malloc(strlen(new_cp) + strlen(neid)+9+1);
        memset(retc, '\0', strlen(new_cp) + strlen(neid)+9+1);
        //printf("%s :: %s >> ", new_cp, NETID);
        strcat(retc, new_cp);
        strcat(retc, " :: ");
        strcat(retc, neid);
        strcat(retc, " >> ");
        return retc;

    } else {
        //printf("%s :: %s >> ", current_path, NETID);
        //char retc[strlen(new_cp) + strlen(neid)+9];
        char* retc = Malloc(strlen(current_path) + strlen(neid)+9+1);
        memset(retc, '\0', strlen(current_path) + strlen(neid)+9+1);
        strcat(retc, current_path);
        strcat(retc, " :: ");
        strcat(retc, neid);
        strcat(retc, " >> ");
        return retc;
    }

    free(current_path);
    return NULL;
}



/******************************************************
 *
 ******************************************************/

void sigchld_handler(int s){
    int status;
    int olderrno = errno;
    // sig_atomic_t oldpid = pid;
    pid = Waitpid(-1, &status, 0);
    /**
     * Cheaking status to see if child is terminated normally.
     **/
    errno = olderrno;
    // pid = oldpid;
}





//int (*builtin_finder[]) (char **) = {&sfish_help, &sfish_exit, &sfish_cd, &sfish_pwd}; // the order corresbonding to builtin_str[] in main.

//return 1 if the shell should be keeping running, 0 for terminiating.
int sfish_execute(int args_c, char** args, char* envp[]){
    if (args[0] == NULL)
    {
        return 1;
    }

    if (strcmp(args[0], "exit") == 0)
    {
        sfish_exit();
    }

    if (strcmp(args[0], "help") == 0)
    {
        sfish_help();
        return 1;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        //printf("\nLOOP-OLDPATH_BEFORE: %s\n", oldpath);
        sfish_cd(args[1]);
        //printf("\nLOOP-OLDPATH_AFTER: %s\n", oldpath);

        return 1;
    }

    if (strcmp(args[0], "pwd") == 0)
    {
        sfish_pwd();
        return 1;
    }

    //laucher below.- lauch other program.
    sigset_t mask, prev;

    Signal(SIGCHLD, sigchld_handler);
    Sigemptyset(&mask);
    Sigaddset(&mask, SIGCHLD);

    // while(1){
        Sigprocmask(SIG_BLOCK, &mask, &prev); //BLOCK SIGCHLD
        if ((pid = Fork()) == 0) //child
        {
            Sigprocmask(SIG_SETMASK, &prev, NULL);//UNBLOCK FOR CHILD
            Execvpe(args[0], args, envp);
            debug("Return from execvpe\n");
        }

        pid = 0;
        /*Parent process*/
        while(!pid){
            sigsuspend(&prev);
        }
        debug("GOT HERE AFTER SIGSUSPEND\n");
        Sigprocmask(SIG_SETMASK, &prev, NULL); // UNBLOCK FOR PARENT
        //pid = getpid();
        // break;
    // }

        return 1;

    }



/*void rediction_pipe_check_set(char** args_buf){

    int i = 0;

    //duplicate args_buf to buffer.
    int args_c = 0;

    //produce argc.
    while(args_buf[args_c] != NULL){
        args_c++;
    }

    int new_argc = args_c;

    //init args, buffer of args_buf.
    char* args[args_c+1];
    for (int i = 0; i < (args_c+1) ; ++i)
    {
        args[i] = NULL;
    }

    // duplicates args_buf to args.
    for (int i = 0; i < args_c; ++i)
    {
        args[i] = args_buf[i]; // args[i] points to what args_buf[i] points to.
    }

    for (int i = 0; i < args_c; ++i)
    {
        args_buf[i] = NULL;
    }

    int counter = 0; // for args_buf
    int index = 0; // for args
    while(args[index] != NULL){
        if (counter == (new_argc - 1))
        {
            new_argc = 2*new_argc;
            args_buf = Realloc(args_buf, new_argc*8);
        }

        if ((strrchr(args[index], '>') == NULL) && (strrchr(args[index], '<') == NULL))
        {
            args_buf[counter] = args[index];
            counter++;
            index++;
        }else{
            if (strrchr(args[index], '>') != NULL)
            {
                char buffer[strlen(args[index])+1];
                memset(buffer, '\0', strlen(args[index])+1 );
                strcpy(buffer, args[index]);
                char* token = strtok(buffer, ">");

            }
        }
    }
}*/
    void rediction_pipe_check_set(char** args_buf){
    //construct a string with space between each argument.
        int string_length = 0;
        int i = 0;
        while(args_buf[i] != NULL){
        string_length = string_length + strlen(args_buf[i])+1; // 1 for space.
        i++;
    }
    char buffer[string_length];
    memset(buffer, '\0', string_length);


}


/*receive readed input and return parsed args[] pointer.
 * Caller should free the mem later.*/
char** parse_input(char* input){
    size_t args_size = 128;
    char** args_buf = Malloc(args_size); //hold 16 pointer in the beginnin. 8 bytes/pointer.
    //memset(args_buf, NULL, args_size);
    //int num = args_size/4;
    for (int i = 0; i < args_size/8; ++i)
    {
        args_buf[i] = NULL;
    }

    //char input_buffer[strlen(input) + 1];

    if (strrchr(input, '|') == NULL)
    {
        /* code */
        if (strstr(input, ">>") != NULL)
        {
            args_buf[1] = INVALID;
            return args_buf;
        }

        if (strstr(input, "<<") != NULL)
        {
            args_buf[1] = INVALID;
            return args_buf;
        }


        int counter = 0;

        char* token = strtok(input, " ");

        while(token != NULL){
            if (counter == (args_size/8-1))
            {
                args_size = args_size + 64;
                args_buf = Realloc(args_buf, args_size);
            }

            if (strstr(token, ">") == NULL && strstr(token, "<") == NULL)
            {
                // token doesn't contain > nor <
                //*(args_buf+counter) = token;
                args_buf[counter] = token;
                counter++;
            }else if(strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
                // token is a single char > or <
                args_buf[counter] = token;
                counter++;
            }else{
                //at least one of > or < appears in token.
                //int symbol_counter = 0;

                char token_buf[strlen(token)+1];
                memset(token_buf, '\0', strlen(token)+1);
                strcpy(token_buf, token);

                char* left_p = strstr(token_buf, "<");
                char* right_p = strstr(token_buf, ">");

                if (left_p != NULL && right_p != NULL)
                {
                    //They are all appears in token. Either > then < , or < then >
                    if(left_p < right_p){
                        // < then >
                        char* token2 = strtok(token_buf, "<");
                        if (strstr(token2, ">") == NULL)
                        {
                            if (left_p < token2)
                            {
                                //case: <A
                                args_buf[counter] = L_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                //case: A<
                                args_buf[counter] = token2;
                                counter++;
                                args_buf[counter] = L_ANGLE;
                                counter++;
                            }
                        }
                        token2 = strtok(NULL, ">");
                        if (strstr(token2, "<") != NULL)
                        {
                            //error
                            args_buf[1] = INVALID;
                            return args_buf;
                        }else{
                            args_buf[counter] = token2;
                            counter++;
                            args_buf[counter] = R_ANGLE;
                            counter++;
                        }

                        if (strstr(token_buf, ">") == NULL && strstr(token_buf, "<") == NULL)
                        {
                            args_buf[counter] = token_buf;
                            counter++;
                        }else{
                            //error
                            args_buf[1] = INVALID;
                            return args_buf;
                        }
                    }else{
                        // > then <
                        char* token2 = strtok(token_buf, ">");
                        if (strstr(token2, "<") == NULL)
                        {
                            if (right_p < token2)
                            {
                                //case: >A
                                args_buf[counter] = R_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                //case: A>
                                args_buf[counter] = token2;
                                counter++;
                                args_buf[counter] = R_ANGLE;
                                counter++;
                            }
                        }
                        token2 = strtok(NULL, "<");
                        if (strstr(token2, ">") != NULL)
                        {
                            //error
                            args_buf[1] = INVALID;
                            return args_buf;
                        }else{
                            args_buf[counter] = token2;
                            counter++;
                            args_buf[counter] = L_ANGLE;
                            counter++;
                        }
                        if (strstr(token_buf, ">") == NULL && strstr(token_buf, "<") == NULL)
                        {
                            args_buf[counter] = token_buf;
                            counter++;
                        }else{
                            //error
                            args_buf[1] = INVALID;
                            return args_buf;
                        }
                    }

                }else if(left_p != NULL && right_p == NULL){
                    char* token2 = strtok(token_buf, "<");

                    if (left_p < token2)
                    {
                                //case: <A
                        args_buf[counter] = L_ANGLE;
                        counter++;
                        args_buf[counter] = token2;
                        counter++;
                    }else{
                                //case: A<
                        args_buf[counter] = token2;
                        counter++;
                        args_buf[counter] = L_ANGLE;
                        counter++;
                    }

                    token2 = strtok(NULL, "<");
                    if (token2 != NULL)
                    {
                        if (strstr(token2, "<") == NULL)
                        {
                            args_buf[counter] = token2;
                            counter++;
                        }else{
                            args_buf[1] = INVALID;
                            return args_buf;
                        }
                    }
                }else if(left_p == NULL && right_p != NULL){
                    char* token2 = strtok(token_buf, ">");

                    if (right_p < token2)
                    {
                                //case: >A
                        args_buf[counter] = R_ANGLE;
                        counter++;
                        args_buf[counter] = token2;
                        counter++;
                    }else{
                                //case: A>
                        args_buf[counter] = token2;
                        counter++;
                        args_buf[counter] = R_ANGLE;
                        counter++;
                    }

                    token2 = strtok(NULL, "<");
                    if (token2 != NULL)
                    {
                        if (strstr(token2, "<") == NULL)
                        {
                            args_buf[counter] = token2;
                            counter++;
                        }else{
                            args_buf[1] = INVALID;
                            return args_buf;
                        }
                    }

                }
            }





            if (strcmp(args_buf[0], "exit") == 0)
            {
                break;
            }
            if (strcmp(args_buf[0], "help")==0)
            {
                break;
            }
            if (strcmp(args_buf[0], "pwd")==0)
            {
                break;
            }
            if ((strcmp(args_buf[0], "cd") == 0) && (counter == 2))
            {
                break;
            }

            token = strtok(NULL, " ");

        }
        args_buf[counter] = NULL;

    }else{
        //now input contains |.
        if (strrchr(input, '>') != NULL)
        {
            args_buf[1] = INVALID;
            return args_buf;
        }

        if (strrchr(input, '<') != NULL)
        {
            args_buf[1] = INVALID;
            return args_buf;
        }

        if (strstr(input, "||") != NULL)
        {
            args_buf[1] = INVALID;
            return args_buf;
        }

        //int num = 0; // counter for "|"
        int counter = 0;

        char* token = strtok(input, " ");

        while(token != NULL){
            if (counter == (args_size/8-1))
            {
                args_size = args_size + 64;
                args_buf = Realloc(args_buf, args_size);
            }

            char* p = strstr(token, "|");

            if (strcmp(token, "|") == 0)
            {
                args_buf[counter] = token;
                counter++;
            }else if(p == NULL){
                args_buf[counter] = token;
                counter++;
            }else{

                char token_buf[strlen(token)+1];
                memset(token_buf, '\0', strlen(token)+1);
                strcpy(token_buf, token);

                char first = (char)(*(token_buf));
                char last = (char)(*(token_buf + strlen(token_buf)-1));
                if (first == '|')
                {
                    args_buf[counter] = S_PIPE;
                    counter++;
                }

                char* token2 = strtok(token_buf, "|");
                int times = 0;
                while(token2!=NULL){
                    if ((counter+times) == (args_size/8-1))
                    {
                        args_size = args_size + 64;
                        args_buf = Realloc(args_buf, args_size);
                    }
                    args_buf[counter] = token2;
                    counter++;
                    args_buf[counter] = S_PIPE;
                    counter++;
                    token2 = strtok(NULL, "|");
                }
                if (last != '|')
                {
                    counter--;
                    args_buf[counter] = NULL;
                    counter++;
                }
            }



            if (strcmp(args_buf[0], "exit") == 0)
            {
                break;
            }
            if (strcmp(args_buf[0], "help")==0)
            {
                break;
            }
            if (strcmp(args_buf[0], "pwd")==0)
            {
                break;
            }
            if ((strcmp(args_buf[0], "cd") == 0) && (counter == 2))
            {
                break;
            }

            token = strtok(NULL, " ");
        }

        args_buf[counter] = NULL;



    }



    return args_buf;
}


int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;
    //char* builtin_str[] = {"help", "exit", "cd", "pwd"};
    char** args_temp;
    memset(oldpath, '\0', 4096);
    pid = getpid();


    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {

        char* cur_path_t = print_prompt();
        char cur_path[strlen(cur_path_t)+1];
        memset(cur_path, '\0', strlen(cur_path_t)+1 );
        strcpy(cur_path, cur_path_t);
        free(cur_path_t);


        input = readline(cur_path);

        /*write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));
        */

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }

        //args_temp = parse_input(input); //need to free this later.
        /************
         *************/
        size_t args_size = 128;
    char** args_buf = Malloc(args_size); //hold 16 pointer in the beginnin. 8 bytes/pointer.
    //memset(args_buf, NULL, args_size);
    //int num = args_size/4;
    for (int i = 0; i < args_size/8; ++i)
    {
        args_buf[i] = NULL;
    }

    //char input_buffer[strlen(input) + 1];

    if (strrchr(input, '|') == NULL)
    {
        /* code */
        if (strstr(input, ">>") != NULL)
        {
            args_buf[1] = INVALID;
            //return args_buf;
            break;
        }

        if (strstr(input, "<<") != NULL)
        {
            args_buf[1] = INVALID;
            //return args_buf;
            break;
        }


        int counter = 0;

        char* token = strtok(input, " ");

        while(token != NULL){
            if (counter == (args_size/8-1))
            {
                args_size = args_size + 64;
                args_buf = Realloc(args_buf, args_size);
            }

            if (strstr(token, ">") == NULL && strstr(token, "<") == NULL)
            {
                // token doesn't contain > nor <
                //*(args_buf+counter) = token;
                args_buf[counter] = token;
                counter++;
            }else if(strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
                // token is a single char > or <
                args_buf[counter] = token;
                counter++;
            }else{
                //at least one of > or < appears in token.
                //int symbol_counter = 0;

                char token_buf[strlen(token)+1];
                memset(token_buf, '\0', strlen(token)+1);
                strcpy(token_buf, token);

                char* left_p = strstr(token_buf, "<");
                char* right_p = strstr(token_buf, ">");

                if (left_p != NULL && right_p != NULL)
                {
                    //They are all appears in token. Either > then < , or < then >
                    if(left_p < right_p){
                        // < then >
                        char* token2 = strtok(token_buf, "<");
                        if (strstr(token2, ">") == NULL)
                        {
                            if (left_p < token2)
                            {
                                //case: <A
                                args_buf[counter] = L_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                //case: A<
                                args_buf[counter] = token2;
                                counter++;
                                args_buf[counter] = L_ANGLE;
                                counter++;
                            }
                        }
                        token2 = strtok(NULL, ">");
                        if (strstr(token2, "<") != NULL)
                        {
                            //error
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }else{
                            args_buf[counter] = token2;
                            counter++;
                            args_buf[counter] = R_ANGLE;
                            counter++;
                        }

                        if (strstr(token_buf, ">") == NULL && strstr(token_buf, "<") == NULL)
                        {
                            args_buf[counter] = token_buf;
                            counter++;
                        }else{
                            //error
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }
                    }else{
                        // > then <
                        char* token2 = strtok(token_buf, ">");
                        if (strstr(token2, "<") == NULL)
                        {
                            if (right_p < token2)
                            {
                                //case: >A
                                args_buf[counter] = R_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                //case: A>
                                args_buf[counter] = token2;
                                counter++;
                                args_buf[counter] = R_ANGLE;
                                counter++;
                            }
                        }
                        token2 = strtok(NULL, "<");
                        if (strstr(token2, ">") != NULL)
                        {
                            //error
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }else{
                            args_buf[counter] = token2;
                            counter++;
                            args_buf[counter] = L_ANGLE;
                            counter++;
                        }
                        if (strstr(token_buf, ">") == NULL && strstr(token_buf, "<") == NULL)
                        {
                            args_buf[counter] = token_buf;
                            counter++;
                        }else{
                            //error
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }
                    }

                }else if(left_p != NULL && right_p == NULL){
                    char* token2 = strtok(token_buf, "<");

                    if (left_p < token2)
                    {
                                //case: <A
                        args_buf[counter] = L_ANGLE;
                        counter++;
                        args_buf[counter] = token2;
                        counter++;
                    }else{
                                //case: A<
                        args_buf[counter] = token2;
                        counter++;
                        args_buf[counter] = L_ANGLE;
                        counter++;
                    }

                    token2 = strtok(NULL, "<");
                    if (token2 != NULL)
                    {
                        if (strstr(token2, "<") == NULL)
                        {
                            args_buf[counter] = token2;
                            counter++;
                        }else{
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }
                    }
                }else if(left_p == NULL && right_p != NULL){
                    char* token2 = strtok(token_buf, ">");

                    if (right_p < token2)
                    {
                                //case: >A
                        args_buf[counter] = R_ANGLE;
                        counter++;
                        args_buf[counter] = token2;
                        counter++;
                    }else{
                                //case: A>
                        args_buf[counter] = token2;
                        counter++;
                        args_buf[counter] = R_ANGLE;
                        counter++;
                    }

                    token2 = strtok(NULL, "<");
                    if (token2 != NULL)
                    {
                        if (strstr(token2, "<") == NULL)
                        {
                            args_buf[counter] = token2;
                            counter++;
                        }else{
                            args_buf[1] = INVALID;
                            //return args_buf;
                            break;
                        }
                    }

                }
            }





            if (strcmp(args_buf[0], "exit") == 0)
            {
                break;
            }
            if (strcmp(args_buf[0], "help")==0)
            {
                break;
            }
            if (strcmp(args_buf[0], "pwd")==0)
            {
                break;
            }
            if ((strcmp(args_buf[0], "cd") == 0) && (counter == 2))
            {
                break;
            }

            token = strtok(NULL, " ");

        }
        args_buf[counter] = NULL;

    }else{
        //now input contains |.
        if (strrchr(input, '>') != NULL)
        {
            args_buf[1] = INVALID;
            //return args_buf;
            break;
        }

        if (strrchr(input, '<') != NULL)
        {
            args_buf[1] = INVALID;
            //return args_buf;
            break;
        }

        if (strstr(input, "||") != NULL)
        {
            args_buf[1] = INVALID;
            //return args_buf;
            break;
        }

        //int num = 0; // counter for "|"
        int counter = 0;

        char* token = strtok(input, " ");

        while(token != NULL){
            if (counter == (args_size/8-1))
            {
                args_size = args_size + 64;
                args_buf = Realloc(args_buf, args_size);
            }

            char* p = strstr(token, "|");

            if (strcmp(token, "|") == 0)
            {
                args_buf[counter] = token;
                counter++;
            }else if(p == NULL){
                args_buf[counter] = token;
                counter++;
            }else{

                char token_buf[strlen(token)+1];
                memset(token_buf, '\0', strlen(token)+1);
                strcpy(token_buf, token);

                char first = (char)(*(token_buf));
                char last = (char)(*(token_buf + strlen(token_buf)-1));
                if (first == '|')
                {
                    args_buf[counter] = S_PIPE;
                    counter++;
                }

                char* token2 = strtok(token_buf, "|");
                int times = 0;
                while(token2!=NULL){
                    if ((counter+times) == (args_size/8-1))
                    {
                        args_size = args_size + 64;
                        args_buf = Realloc(args_buf, args_size);
                    }
                    args_buf[counter] = token2;
                    counter++;
                    args_buf[counter] = S_PIPE;
                    counter++;
                    token2 = strtok(NULL, "|");
                }
                if (last != '|')
                {
                    counter--;
                    args_buf[counter] = NULL;
                    counter++;
                }
            }



            if (strcmp(args_buf[0], "exit") == 0)
            {
                break;
            }
            if (strcmp(args_buf[0], "help")==0)
            {
                break;
            }
            if (strcmp(args_buf[0], "pwd")==0)
            {
                break;
            }
            if ((strcmp(args_buf[0], "cd") == 0) && (counter == 2))
            {
                break;
            }

            token = strtok(NULL, " ");
        }

        args_buf[counter] = NULL;

    }

        /************
         *************/
    args_temp = args_buf;

    int args_c = 0;

    while(args_temp[args_c] != NULL){
        printf("%s ", args_temp[args_c]);
        args_c++;

    }
    printf("\n");

    char* args[args_c+1];
    for (int i = 0; i < (args_c+1) ; ++i)
    {
       args[i] = NULL;
   }

   for (int i = 0; i < args_c; ++i)
   {
    args[i] = args_temp[i];
}

free(args_temp);


sfish_execute(args_c, args, envp);

        // You should change exit to a "builtin" for your hw.
        //exited = strcmp(input, "exit") == 0;
        //exited = strcmp(input, builtin_str[1]) == 0;


        // Readline mallocs the space for input. You must free it.
rl_free(input);

} while(!exited);

debug("%s", "user entered 'exit'");

return EXIT_SUCCESS;
}


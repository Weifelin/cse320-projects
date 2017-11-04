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
    int fd[2]={STDIN_FILENO, STDOUT_FILENO};

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


            Sigprocmask(SIG_BLOCK, &mask, &prev); //BLOCK SIGCHLD
            if ((pid = Fork()) == 0) //child
            {
                Sigprocmask(SIG_SETMASK, &prev, NULL);//UNBLOCK FOR CHILD
                //reading.
                //close(fd[1]);
                Execvpe(args[0], args, envp);
                debug("Return from execvpe\n");
            }

            pid = 0;
            /*Parent process*/
            while(!pid){
                sigsuspend(&prev);
            }
            //debug("GOT HERE AFTER SIGSUSPEND\n");
            Sigprocmask(SIG_SETMASK, &prev, NULL); // UNBLOCK FOR PARENT
            //pid = getpid();
            return 1;

    }


    int sfish_execute_r(int args_c, char** args, char* envp[]){
        FILE* in_stream = stdin;
        FILE* out_stream = stdout;
        char* infile = NULL;
        char* outfile = NULL;

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


        //determine mode.
            //int left = array_contain_count(args, "<", args_c);
            //int right = array_contain_count(args, ">", args_c);
        //int pipe_sign = array_contain_count(args, "|", args_c);
        int left_i = get_first_id(args, "<", args_c);
        int right_i = get_first_id(args, ">", args_c);

        if(left_i != -1)
            infile = args[left_i+1];
        if (right_i != -1)
            outfile = args[right_i+1];

        if (infile != NULL)
        {
            in_stream = Fopen(infile, "r");
            int instream_fd = fileno(in_stream);
            dup2(instream_fd, STDIN_FILENO);
            fd[0] = instream_fd;
            debug("infile");
        }

        if (outfile != NULL)
        {
            out_stream = Fopen(outfile, "w+");
            int outstream_fd = fileno(out_stream);
            dup2(outstream_fd, STDOUT_FILENO);
            fd[1] = outstream_fd;
        }

        pipe(fd);
        //construct args string for child.
        int count = 0;
        while((strcmp(args[count], ">") != 0) && (strcmp(args[count], "<")!= 0)){
            int ret =  write(fd[1], args[count], strlen(args[count]));
            if (ret == -1)
            {
                printf(SYNTAX_ERROR, "Faild to write");
            }
            int ret2 =  write(fd[1]," ", 1);
            if (ret2 == -1)
            {
                printf(SYNTAX_ERROR, "Faild to write space");
            }
            count++;
        }
        int ret_f = write(fd[1], "\n", 1);
        if (ret_f == -1)
        {
            printf(SYNTAX_ERROR, "Faild to write newline");
        }


        //laucher below.- lauch other program.
        sigset_t mask, prev;

        Signal(SIGCHLD, sigchld_handler);
        Sigemptyset(&mask);
        Sigaddset(&mask, SIGCHLD);


            Sigprocmask(SIG_BLOCK, &mask, &prev); //BLOCK SIGCHLD
            if ((pid = Fork()) == 0) //child
            {
                Sigprocmask(SIG_SETMASK, &prev, NULL);//UNBLOCK FOR CHILD
                //reading.
                //close(fd[1]);
                //dup2(fd[0], STDIN_FILENO);
                FILE* readfrom = Fdopen(fd[0], "r");
                size_t buffersize = 512;
                char* input_buffer = Malloc(buffersize);
                memset(input_buffer, '\0', buffersize);
                int c;
                int count=0;

                while((c = getc(readfrom)) != '\n'){
                    if (count == (buffersize-1))
                    {
                        buffersize=2*buffersize;
                        input_buffer = Realloc(input_buffer, buffersize);
                    }

                    input_buffer[count] = (char) c;
                    count++;
                }

                if (input_buffer[0] == '\0')
                {
                    exit(1);
                }

                debug("Child-input_buffer: %s", input_buffer);

                char input[strlen(input_buffer)+1];
                for (int i = 0; i < (strlen(input_buffer)+1); ++i)
                {
                    input[i] = input_buffer[i];
                }
                debug("Child-input_buffer: %s", input);
                char** args_temp = parse_input(input); //need to free this later.
                free(input_buffer);

                if (args_temp == NULL)
                {
                    exit(1);
                }
                int args_cu = 0;

                while(args_temp[args_cu] != NULL){
                    debug("child:%s, length: %i", args_temp[args_cu], (int)strlen(args_temp[args_cu]));
                    args_cu++;
                }

                debug("\n");

                char* args_child[args_cu+1];
                for (int i = 0; i < (args_cu+1) ; ++i)
                {
                    args_child[i] = NULL;
                }

                for (int i = 0; i < args_cu; ++i)
                {
                    args_child[i] = args_temp[i];
                }

                free(args_temp);

                Execvpe(args_child[0], args_child, envp);
                dup2(fileno(stdout), fd[1]);
                close(fd[1]);
                debug("Return from execvpe\n");
            }

            pid = 0;
            /*Parent process*/
            //close(fd[0]);

            while(!pid){
                sigsuspend(&prev);
            }
            //debug("GOT HERE AFTER SIGSUSPEND\n");
            Sigprocmask(SIG_SETMASK, &prev, NULL); // UNBLOCK FOR PARENT
            //pid = getpid();

            //dup2(STDIN_FILENO, fd[0]);
            //dup2(STDOUT_FILENO, fd[1]);
            // dup2(STDIN_FILENO, fileno(in_stream));
            //Fdopen(STDIN_FILENO);
            //Fdopen(STDOUT_FILENO);
            //dup2(fileno(stdin), fd[0]);
            //dup2(fileno(stdout), fd[1]);
            //close(fd[0]);
            close(fd[0]);


            debug("reaching here.");
            //Fclose(in_stream);
            //Fclose(out_stream);


            return 1;

    }

    int sfish_execute_p(int args_c, char** args, char* envp[]){
        FILE* in_stream = stdin;
        FILE* out_stream = stdout;
        int fd[2] = {STDIN_FILENO, STDOUT_FILENO};
        char* infile;
        char* outfile;

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


        //determine mode.

            //int left = array_contain_count(args, "<", args_c);
            //int right = array_contain_count(args, ">", args_c);
            //int pipe_sign = array_contain_count(args, "|", args_c);
            int left_i = get_first_id(args, "<", args_c);
            int right_i = get_first_id(args, ">", args_c);


                if(left_i != -1)
                    infile = args[left_i+1];
                if (right_i != -1)
                    outfile = args[left_i+1];

                if (infile != NULL)
                {
                    in_stream = Fopen(infile, "r");
                    int instream_fd = fileno(in_stream);
                    dup2(instream_fd, STDIN_FILENO);
                    fd[0] = instream_fd;
                    debug("infile");
                }

                if (outfile != NULL)
                {
                    out_stream = Fopen(outfile, "w+");
                    int outstream_fd = fileno(out_stream);
                    dup2(outstream_fd, STDOUT_FILENO);
                    fd[1] = outstream_fd;
                }

                pipe(fd);



        //laucher below.- lauch other program.
        sigset_t mask, prev;

        Signal(SIGCHLD, sigchld_handler);
        Sigemptyset(&mask);
        Sigaddset(&mask, SIGCHLD);


            Sigprocmask(SIG_BLOCK, &mask, &prev); //BLOCK SIGCHLD
            if ((pid = Fork()) == 0) //child
            {
                Sigprocmask(SIG_SETMASK, &prev, NULL);//UNBLOCK FOR CHILD
                //reading.
                close(fd[1]);
                Execvpe(args[0], args, envp);
                debug("Return from execvpe\n");
            }

            pid = 0;
            /*Parent process*/
            close(fd[0]);
            while(!pid){
                sigsuspend(&prev);
            }
            //debug("GOT HERE AFTER SIGSUSPEND\n");
            Sigprocmask(SIG_SETMASK, &prev, NULL); // UNBLOCK FOR PARENT
            //pid = getpid();
            dup2(STDOUT_FILENO, fileno(out_stream));
            dup2(STDIN_FILENO, fileno(in_stream));

                //Fclose(in_stream);
                //Fclose(out_stream);


            return 1;

        }
    /*receive readed input and return parsed args[] pointer.
     * Caller should free the mem later.*/
    char** parse_input(char* input){
        if (input == NULL)
        {
            return NULL;
        }
        if (strncmp(input, "", 1) == 0)
        {
            return NULL;
        }
        size_t args_size = 128;
        char** args_buf = Malloc(args_size); //hold 16 pointer in the beginnin. 8 bytes/pointer.
        //memset(args_buf, NULL, args_size);
        //int num = args_size/4;
        for (int i = 0; i < args_size/8; ++i)
        {
            args_buf[i] = NULL;
        }
        char* savepter;

        //char input_buffer[strlen(input) + 1];

        if (strrchr(input, '|') == NULL)
        {
            /* code */
            if (strstr(input, ">>") != NULL)
            {
                args_buf[0] = INVALID;
                return args_buf;
            }

            if (strstr(input, "<<") != NULL)
            {
                args_buf[0] = INVALID;
                return args_buf;
            }


            int counter = 0;

            char* token = strtok_r(input, " ", &savepter);

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

                    // char token_buf[strlen(token)+1];
                    // memset(token_buf, '\0', strlen(token)+1);
                    // strcpy(token_buf, token);
                    char* token_buf = token;

                    char* left_p = strstr(token_buf, "<");
                    char* right_p = strstr(token_buf, ">");

                    char first = (char) *token_buf;


                    char last = (char) *(token_buf + strlen(token_buf) -1);
                    char* last_p = token_buf + strlen(token_buf) -1;

                    if (left_p != NULL && right_p != NULL)
                    {
                        //They are all appears in token. Either > then < , or < then >
                        if (left_p < right_p)
                        {
                            //case: < then >
                            /*if (first == '<' && last != '>')
                            {
                                args_buf[counter] = L_ANGLE;
                                counter++;
                            }*/

                            char* token2 = strtok(token_buf, "<");

                            if (strstr(token2, "<") != NULL)
                            {
                                args_buf[0] = INVALID;
                                return args_buf;
                            }

                            if (strstr(token2, ">") == NULL) //case: arg < ....
                            {
                                args_buf[counter] = token2;
                                counter++;
                            }
                            //token2 contains >

                            //token2 = strtok(NULL, ">");
                            //in case <INPUT>, token2 will be null.
                            //token2 = strtok(token2, ">");
                            /*if (first == '>')
                            {
                                token2 = strtok(token2, ">");
                            }else{
                                token2 = strtok(NULL, ">");
                            }*/
                            if (first != '<')
                            {
                                token2 = strtok(NULL, "<");
                            }else{
                                token2 = strtok(token2, ">");
                            }

                            if (token2 != NULL)
                            {
                                if (left_p < token2)
                                {
                                    args_buf[counter] = L_ANGLE;
                                    counter++;
                                }
                                if (strstr(token2, ">") != NULL)
                                {
                                    char* savepter3;
                                    char* token4 = strtok_r(token2, ">", &savepter3);
                                    if (token4!= NULL)
                                    {
                                        args_buf[counter] = token4;
                                        counter++;
                                        //token2 = savepter3;
                                    }

                                    if (savepter3 != NULL)
                                    {
                                        if (strstr(savepter3, ">") != NULL)
                                        {
                                            args_buf[0] = INVALID;
                                            return args_buf;
                                        }
                                        if (strncmp(savepter3, "", 1) != 0 && right_p < savepter3)
                                        {
                                            args_buf[counter] = R_ANGLE;
                                            counter++;
                                            args_buf[counter] = savepter3;
                                            counter++;
                                        }
                                    }


                                }else{
                                    args_buf[counter] = token2;
                                    counter++;
                                }
                            }

                            /*if (token2 == NULL && last == '>')
                            {
                                char* savepter2;
                                char* token3 = strtok_r(token_buf, ">", &savepter2);
                                if (token3!=NULL && first == '<')
                                {
                                    if (left_p == token3)
                                    {
                                        token3 = strtok_r(token3, "<", &savepter2);
                                        if (token3 != NULL)
                                        {
                                            args_buf[counter] = token3;
                                            counter++;
                                        }
                                    }
                                }

                            }*/

                            token2 = strtok(NULL, ">");
                            if (token2 != NULL)
                            {
                                if (token2 < last_p && last == '>')
                                {
                                    args_buf[0] = INVALID;
                                    return args_buf;
                                }
                                if (right_p < token2)
                                {
                                    args_buf[counter] = R_ANGLE;
                                    counter++;
                                }
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                if (last == '>')
                                {
                                    args_buf[counter] = R_ANGLE;
                                    counter++;
                                }
                            }
                            token2 = strtok(NULL, ">");
                            if (token2 != NULL)
                            {
                                args_buf[0] = INVALID;
                                return args_buf;
                            }




                        }else{
                            //csdr: > then <
                            char* token2 = strtok(token_buf, ">");

                            if (strstr(token2, ">") != NULL)
                            {
                                args_buf[0] = INVALID;
                                return args_buf;
                            }

                            if (strstr(token2, "<") == NULL) //case: arg > ....
                            {
                                args_buf[counter] = token2;
                                counter++;
                            }
                            //token2 contains >
                            //token2 = strtok(NULL, "<");
                            /*if (first == '>')
                            {
                                token2 = strtok(token2, "<");
                            }else{
                                token2 = strtok(NULL, "<");
                            }*/
                            //token2 = strtok(token2, "<");
                            if (first != '>')
                            {
                                token2 = strtok(NULL, ">");
                            }else{
                                token2 = strtok(token2, "<");
                            }

                            if (token2 != NULL)
                            {
                                if (right_p < token2)
                                {
                                    args_buf[counter] = R_ANGLE;
                                    counter++;
                                }
                                if (strstr(token2, "<") != NULL)
                                {
                                    char* savepter3;
                                    char* token4 = strtok_r(token2, "<", &savepter3);
                                    if (token4!= NULL)
                                    {
                                        args_buf[counter] = token4;
                                        counter++;
                                        //token2 = savepter3;
                                    }


                                    if (savepter3 != NULL)
                                    {
                                        if (strstr(savepter3, "<") != NULL)
                                        {
                                            args_buf[0] = INVALID;
                                            return args_buf;
                                        }

                                        if (strncmp(savepter3, "", 1) != 0 && left_p < savepter3)
                                        {
                                            args_buf[counter] = L_ANGLE;
                                            counter++;
                                            args_buf[counter] = savepter3;
                                            counter++;
                                        }
                                    }


                                }else{
                                    args_buf[counter] = token2;
                                    counter++;
                                }
                            }

                            /*if (token2 == NULL && last == '<')
                            {
                                char* savepter2;
                                char* token3 = strtok_r(token_buf, "<", &savepter2);
                                if (token3!=NULL && first == '>')
                                {
                                    if (right_p == token3)
                                    {
                                        token3 = strtok_r(token3, ">", &savepter2);
                                        if (token3 != NULL)
                                        {
                                            args_buf[counter] = token3;
                                            counter++;
                                        }
                                    }
                                }

                            }*/

                            token2 = strtok(NULL, "<");
                            if (token2 != NULL)
                            {
                                if (token2 < last_p && last == '<')
                                {
                                    args_buf[0] = INVALID;
                                    return args_buf;
                                }
                                if (left_p < token2)
                                {
                                    args_buf[counter] = L_ANGLE;
                                    counter++;
                                }
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                if (last == '<')
                                {
                                    args_buf[counter] = L_ANGLE;
                                    counter++;
                                }
                            }
                            token2 = strtok(NULL, "<");
                            if (token2 != NULL)
                            {
                                args_buf[0] = INVALID;
                                return args_buf;
                            }
                        }

                    }else if(left_p != NULL && right_p == NULL){

                        if (first == '>')
                        {
                            args_buf[counter] = R_ANGLE;
                            counter++;
                        }
                        if (first == '<')
                        {
                            args_buf[counter] = L_ANGLE;
                            counter++;
                        }

                        char* token2 = strtok(token_buf, "<");

                        if (strstr(token2, "<") != NULL)
                        {
                            args_buf[0] = INVALID;
                            return args_buf;
                        }

                        args_buf[counter] = token2;
                        counter++;

                        //check last
                        token2 = strtok(NULL, "<");

                        if (token2!=NULL)
                        {
                            if (first != '<' && left_p < token2)
                            {
                                //valid.
                                args_buf[counter] = L_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                args_buf[0] = INVALID;
                                return args_buf;
                            }

                        }else{
                            if (last=='<')
                            {
                                args_buf[counter] = L_ANGLE;
                                counter++;
                            }
                        }
                    }else if(left_p == NULL && right_p != NULL){

                        if (first == '>')
                        {
                            args_buf[counter] = R_ANGLE;
                            counter++;
                        }
                        if (first == '<')
                        {
                            args_buf[counter] = L_ANGLE;
                            counter++;
                        }
                        char* token2 = strtok(token_buf, ">");

                        if (strstr(token2, ">") != NULL)
                        {
                            args_buf[0] = INVALID;
                            return args_buf;
                        }

                        args_buf[counter] = token2;
                        counter++;

                        //check last
                        token2 = strtok(NULL, ">");
                        if (token2!=NULL)
                        {
                            if (first != '<' && right_p < token2)
                            {
                                //valid.
                                args_buf[counter] = R_ANGLE;
                                counter++;
                                args_buf[counter] = token2;
                                counter++;
                            }else{
                                args_buf[0] = INVALID;
                                return args_buf;
                            }
                        }{
                            if (last=='>')
                            {
                                args_buf[counter] = R_ANGLE;
                                counter++;
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

                token = strtok_r(NULL, " ", &savepter);

            }
            args_buf[counter] = NULL;

        }else{
            //now input contains |.
            if (strrchr(input, '>') != NULL)
            {
                args_buf[0] = INVALID;
                return args_buf;
            }

            if (strrchr(input, '<') != NULL)
            {
                args_buf[0] = INVALID;
                return args_buf;
            }

            if (strstr(input, "||") != NULL)
            {
                args_buf[0] = INVALID;
                return args_buf;
            }

            //int num = 0; // counter for "|"
            int counter = 0;

            char* token = strtok_r(input, " ", &savepter);

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

                    // char token_buf[strlen(token)+1];
                    // memset(token_buf, '\0', strlen(token)+1);
                    // strcpy(token_buf, token);
                    char* token_buf = token;

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

                token = strtok_r(NULL, " ", &savepter);
            }

            args_buf[counter] = NULL;



        }



        return args_buf;
    }

    // return 0 if it doesn't contain return number of occurance if exits.
    int array_contain_count(char** args_buf, char* string, int argc){
        int count = 0;
        for (int i = 0; i < argc; ++i)
        {
            if (strcmp(args_buf[i], string) == 0)
            {
                count++;
            }
        }
        return count;
    }

    //return -1 if there isn't.
    int get_first_id(char** args_buf, char* string, int argc){
        int j=-1;
        for (int i = 0; i < argc; ++i)
        {
            if (strcmp(args_buf[i], string) == 0)
            {
                return i;
            }
        }

        return j;
    }

    //return -1 if error, 0 on regular. 1 on sucess pipe. 2 on rediction set.
    int args_validate_and_set(char** args_buf){
        int argc = 0;

        if (strcmp(args_buf[0], INVALID) == 0)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        while(args_buf[argc] !=NULL){
            argc++;
        }

        int left = array_contain_count(args_buf, "<", argc);
        int right = array_contain_count(args_buf, ">", argc);
        int pipe_sign = array_contain_count(args_buf, "|", argc);
        int left_i = get_first_id(args_buf, "<", argc);
        int right_i = get_first_id(args_buf, ">", argc);

        if (left==0 && right == 0 && pipe_sign == 0)
        {
            return 0; //regular command.
        }

        if (right > 1 || left > 1)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if ((left != 0)&& ( pipe_sign!=0))
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if ((right!=0)&& (pipe_sign !=0))
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if (strcmp(args_buf[0], "cd")==0 && strcmp(args_buf[1], ">") == 0)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if (strcmp(args_buf[0], "cd")==0 && strcmp(args_buf[1], "<") == 0)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if (strcmp(args_buf[0], "cd")==0 && strcmp(args_buf[1], "|") == 0)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if (strcmp(args_buf[0], "|") == 0 || strcmp(args_buf[0], ">") == 0 || strcmp(args_buf[0], "<") == 0)
        {
            printf(SYNTAX_ERROR, "INVALID INPUT.");
            return -1;
        }

        if (left!=0 && right != 0)
        {

            for (int i = 0; i < argc; ++i)
            {
                if (strcmp(args_buf[i], "<") == 0 || strcmp(args_buf[i], ">") == 0 || strcmp(args_buf[i], "|") == 0)
                {
                    if (args_buf[i+1] == NULL)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }

                    if (strcmp(args_buf[i+1], "<") == 0 || strcmp(args_buf[i+1], ">") == 0 || strcmp(args_buf[i+1], "|") == 0)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }

                }
            }

            if (left_i < right_i)//<>
            {
                if (right_i - left_i != 2)
                {
                    printf(SYNTAX_ERROR, "INVALID INPUT.");
                    return -1;
                }

                if (args_buf[right_i+1] != NULL)
                {
                    if (args_buf[right_i+2] != NULL)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }
                }
            }
            if (left_i > right_i){//><
                if (left_i - right_i !=2)
                {
                    printf(SYNTAX_ERROR, "INVALID INPUT.");
                    return -1;
                }

                if (args_buf[left_i+1] != NULL)
                {
                    if (args_buf[left_i+2] != NULL)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }
                }
            }
        }

        if ((left==0 && right !=0) || (left!=0 && right ==0))
        {
            if (right==0)
            {
                if (args_buf[left_i+1] != NULL)
                {
                    if (args_buf[left_i+2] != NULL)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }
                }
            }
            if (left==0)
            {
                if (args_buf[right_i+1] != NULL)
                {
                    if (args_buf[right_i+2] != NULL)
                    {
                        printf(SYNTAX_ERROR, "INVALID INPUT.");
                        return -1;
                    }
                }
            }
        }

        if (pipe_sign != 0)
        {
            for (int i = 1; i < argc; ++i)
            {

                if (strcmp(args_buf[i], "|")==0 && strcmp(args_buf[i-1], "|")==0 )
                {
                    printf(SYNTAX_ERROR, "INVALID INPUT.");
                    return -1;
                }
            }
        }

        if (pipe_sign == 0)
        {
            if (left != 0 || right != 0)
            {
                return 2;
            }
        }


        return 2;

    }


    int main(int argc, char *argv[], char* envp[]) {
        char* input;
        bool exited = false;
        //char* builtin_str[] = {"help", "exit", "cd", "pwd"};
        char** args_temp;
        memset(oldpath, '\0', 4096);
        pid = getpid();
        // FILE* in_stream = stdin;
        // FILE* out_stream = stdout;


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

            debug("Input: %s", input);
            args_temp = parse_input(input); //need to free this later.
            if (args_temp == NULL)
            {
                continue;
            }
            /************
             *************/

            //args_temp = args_buf;

            int args_c = 0;

            while(args_temp[args_c] != NULL){
                debug("%s, length: %i", args_temp[args_c], (int)strlen(args_temp[args_c]));
                args_c++;

            }
            debug("\n");

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

            int mode = args_validate_and_set(args);
            debug("mode: %i", mode);
            if (mode == -1)
            {
                continue;
            }


            if (mode == 0)
            {
                sfish_execute(args_c, args, envp);
            }else if (mode == 1) //piping
            {
                sfish_execute_p(args_c, args, envp);

            }else if(mode == 2){ //redirection
                sfish_execute_r(args_c, args, envp);
            }


            // You should change exit to a "builtin" for your hw.
            //exited = strcmp(input, "exit") == 0;
            //exited = strcmp(input, builtin_str[1]) == 0;


            // Readline mallocs the space for input. You must free it.
            rl_free(input);

        } while(!exited);

        debug("%s", "user entered 'exit'");

        return EXIT_SUCCESS;
    }


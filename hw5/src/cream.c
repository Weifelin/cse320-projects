//#include "cream.h"
//#include "utils.h"
#include "hw5p3.h"
#include "csapp.h"
#include "debug.h"



int main(int argc, char *argv[]) {
    //./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES
    args_t* args;
    int exit_code = EXIT_SUCCESS;

    args = parse_args(argc, argv);
    if (args == NULL)
    {
        app_error("invalid argument amount.");// this will exit the program.
    }

    //do something here.
    //queue_and_hashmap_t* queue_and_hashmap;
    queue_t* queue = create_queue();
    hashmap_t* hashmap = create_map(args->max_entries, jenkins_one_at_a_time_hash, map_destroy_function);
    //queue_and_hashmap->queue = queue;
    //queue_and_hashmap->hashmap = hashmap;
    queue_and_hashmap_t queue_and_hashmap = {queue, hashmap};

    int listenfd;
    char* port_number = args -> port_number;
    socklen_t clientlen;
    struct sockaddr_storage client_addr;
    pthread_t thread_ids[args->number_workers];

    listenfd = Open_listenfd(port_number);

    for (int i = 0; i < args->number_workers; ++i)
    {
        Pthread_create(&thread_ids[i], NULL, thread_routine, &queue_and_hashmap);
    }

    while(true){
        clientlen = sizeof(struct sockaddr_storage);
        int* connected_fd = Malloc(sizeof(int)); // need to be freed for invalidate queue.
        *connected_fd = Accept(listenfd, (SA *)&client_addr, &clientlen);//
        enqueue(queue, (void*)connected_fd);
    }

    //--------------------------------------------------
    int number_workers = args->number_workers;
    Free(args->port_number);
    Free(args);
    item_destructor_f destory_function =  &queue_destroy_function;
    invalidate_queue(queue, destory_function);


    for(int index = 0; number_workers; index++) {
        Pthread_join(thread_ids[index], NULL);
    }


    exit(exit_code);


}

void* thread_routine(void* args){ // args is queue.

    while(true){
        queue_t* queue = ((queue_and_hashmap_t*) args) -> queue;
        hashmap_t* hashmap = ((queue_and_hashmap_t*) args) -> hashmap;
        //static pthread_once_t once_control = PTHREAD_ONCE_INIT;
        //Pthread_once(&once_control, )
        request_header_t request_header;
        response_header_t response_header;
        int connected_fd = *((int*)dequeue(queue));

        signal(SIGPIPE, sigpipe_handler);


        Rio_readn(connected_fd, &request_header, sizeof(request_header));

        if (errno == EINTR || errno == EPIPE)
        {
            Close(connected_fd);
            continue;
        }

        if (request_header.request_code == PUT)
        {
            uint32_t key_size = request_header.key_size;
            uint32_t value_size = request_header.value_size;
            if (keysize_valid(key_size) == false || valuesize_valid(value_size) == false)
            {
                //error case. BAD_REQUEST, value_size = 0;
                //response_header ={ BAD_REQUEST, 0};
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connected_fd, &response_header, sizeof(response_header));
                Close(connected_fd);
            }else{

                //int buf_size = key_size+value_size+sizeof(request_header)+2;
                //void* buf_temp = Calloc(1, buf_size);

                // Rio_readn(connected_fd, buf_temp, buf_size);
                // if (errno == EINTR || errno == EPIPE)
                // {
                //     Close(connected_fd);
                //     continue;
                // }

                // void* key_base_t = buf_temp+sizeof(request_header);
                // void* val_base_t = key_base_t + key_size;

                void* key_base = Calloc(1, key_size); // these two needs to be freed when invalidate map.
                void* val_base = Calloc(1, value_size);

                // memcpy(key_base, key_base_t, key_size);
                // memcpy(val_base, val_base_t, value_size);

                Rio_readn(connected_fd, key_base, key_size);
                if (errno == EINTR || errno == EPIPE)
                {
                    Close(connected_fd);
                    continue;
                }
                Rio_readn(connected_fd, val_base, value_size);
                if (errno == EINTR || errno == EPIPE)
                {
                    Close(connected_fd);
                    continue;
                }

                //debug
                //printf("%s\n", (char*)key_base);
                //printf("%s\n", (char*)val_base);

                //Free(buf_temp);

                map_key_t key = MAP_KEY(key_base, key_size);
                map_val_t val = MAP_VAL(val_base, value_size);
                if (put(hashmap, key, val, true) == true)
                {
                    //response_header = {OK, value_size};
                    response_header.response_code = OK;
                    response_header.value_size = value_size;
                    Rio_writen(connected_fd, &response_header, sizeof(response_header));
                    Close(connected_fd);
                }else{
                    //response_header ={BAD_REQUEST, 0};
                    response_header.response_code = BAD_REQUEST;
                    response_header.value_size = 0;
                    Rio_writen(connected_fd, &response_header, sizeof(response_header));
                    Close(connected_fd);
                }
            }


        }else if(request_header.request_code == GET){

            uint32_t key_size = request_header.key_size;
            if (keysize_valid(key_size) == false )
            {
                //error case. BAD_REQUEST, value_size = 0;
                //response_header ={BAD_REQUEST, 0};
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connected_fd, &response_header, sizeof(response_header));
                Close(connected_fd);
            }else{

                // int buf_size = key_size+sizeof(request_header)+1;
                // void* buf_temp = Calloc(1, buf_size);

                // Rio_readn(connected_fd, buf_temp, buf_size);
                // if (errno == EINTR || errno == EPIPE)
                // {
                //     Close(connected_fd);
                //     continue;
                // }

                // void* key_base_t = buf_temp+sizeof(request_header);

                void* key_base = Calloc(1, key_size);
                // memcpy(key_base, key_base_t, key_size);
                // Free(buf_temp);
                Rio_readn(connected_fd, key_base, key_size);
                if (errno == EINTR || errno == EPIPE)
                {
                    Close(connected_fd);
                    continue;
                }

                map_key_t key = MAP_KEY(key_base, key_size);

                map_val_t val = get(hashmap, key);

                if (val.val_base != NULL)
                {
                    //response_header = {OK, val.val_len};
                    response_header.response_code = OK;
                    response_header.value_size = val.val_len;
                    Rio_writen(connected_fd, &response_header, sizeof(response_header));
                    if (errno == EINTR || errno == EPIPE)
                    {
                        Close(connected_fd);
                        continue;
                    }
                    Rio_writen(connected_fd, val.val_base, val.val_len);
                    if (errno == EINTR || errno == EPIPE)
                    {
                        Close(connected_fd);
                        continue;
                    }
                    Close(connected_fd);
                }else{
                    //response_header = {NOT_FOUND, 0};
                    response_header.response_code = NOT_FOUND;
                    response_header.value_size = 0;
                    Rio_writen(connected_fd, &response_header, sizeof(response_header));
                    Close(connected_fd);
                }


            }

        }else if(request_header.request_code == EVICT){
            uint32_t key_size = request_header.key_size;
            if (keysize_valid(key_size) == false )
            {

                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
                Rio_writen(connected_fd, &response_header, sizeof(response_header));
                Close(connected_fd);
            }else{
                // int buf_size = key_size+sizeof(request_header)+1;
                // void* buf_temp = Calloc(1, buf_size);

                // Rio_readn(connected_fd, buf_temp, buf_size);
                // if (errno == EINTR || errno == EPIPE)
                // {
                //     Close(connected_fd);
                //     continue;
                // }

                //void* key_base_t = buf_temp+sizeof(request_header);

                void* key_base = Calloc(1, key_size);
                // memcpy(key_base, key_base_t, key_size);
                // Free(buf_temp);
                Rio_readn(connected_fd, key_base, key_size);
                if (errno == EINTR || errno == EPIPE)
                {
                    Close(connected_fd);
                    continue;
                }

                map_key_t key = MAP_KEY(key_base, key_size);

                delete(hashmap, key);

                response_header.response_code = OK;
                response_header.value_size = 0;
                Rio_writen(connected_fd, &response_header, sizeof(response_header));
                Close(connected_fd);

            }

        }else if(request_header.request_code == CLEAR){
            clear_map(hashmap);
            response_header.response_code = OK;
            response_header.value_size = 0;
            Rio_writen(connected_fd, &response_header, sizeof(response_header));
            Close(connected_fd);
        }else{
            //not supported request code.
            response_header.response_code = UNSUPPORTED;
            response_header.value_size = 0;
            Rio_writen(connected_fd, &response_header, sizeof(response_header));
            Close(connected_fd);
        }



    }//while tail





    return NULL;
}

void map_destroy_function(map_key_t key, map_val_t val){
    free(key.key_base);
    free(val.val_base);
}
void queue_destroy_function(void *item){
    free(item);
}

args_t *parse_args(int argc, char **argv){
    if (argc == 1)
    {
        return NULL;
    }

    if (argc != 4 && argc >= 2)
    {
        if (strcmp(argv[1],"-h") == 0)
        {
            USAGE(*argv, EXIT_SUCCESS);
        }
        //debug("argv[1] is %s, Not -h but argc != 4", argv[1]);
        return NULL;
    }

    args_t *args = Malloc(sizeof(args_t));
    int number_workers = atoi(argv[1]);
    int max_entries = atoi(argv[3]);
    if (number_workers == 0 || max_entries == 0)
    {
        app_error("invalid argument.");
    }
    args->number_workers = number_workers;
    args->port_number = strdup(argv[2]);
    args->max_entries = max_entries;

    return args;
}

bool keysize_valid(uint32_t key_size){
    if (key_size < MIN_KEY_SIZE || key_size > MAX_KEY_SIZE)
    {
        return false;
    }

    return true;
}
bool valuesize_valid(uint32_t value_size){
    if (value_size < MIN_VALUE_SIZE || value_size > MAX_VALUE_SIZE)
    {
        return false;
    }

    return true;
}

void sigpipe_handler(int s){
    app_error("SIGPIPE catched.");
}



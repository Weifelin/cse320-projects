#ifndef HW5P3_H
#define HW5P3_H

#include "cream.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "utils.h"
//#include "hashmap.h"
#include "queue.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

typedef struct args_t {
    int number_workers;
    char* port_number;
    int max_entries;
} args_t;

typedef struct queue_and_hashmap_t {
    queue_t *queue;
    hashmap_t *hashmap;
} queue_and_hashmap_t;


args_t *parse_args(int argc, char **argv);
void* thread_routine(void* args);

void map_destroy_function(map_key_t key, map_val_t val);
void queue_destroy_function(void *item);

bool keysize_valid(uint32_t key_size);
bool valuesize_valid(uint32_t value_size);

void sigpipe_handler(int s);


























#define USAGE(program_name, retcode) do{                                                        \
fprintf(stderr, "USAGE: %s %s\n", program_name,                                                         \
"[-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n" \
"-h                 Displays this help menu and returns EXIT_SUCCESS.\n"                                              \
"NUM_WORKERS        The number of worker threads used to service requests.\n"        \
"PORT_NUMBER        Port number to listen on for incoming connections.\n"          \
"MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store."          \
);                                          \
exit(retcode);                                                                    \
} while(0)

#endif
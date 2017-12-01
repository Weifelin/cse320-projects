//#include "utils.h"
#include "hw5p2.h"
#include "csapp.h"
#include "debug.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

bool key_compare(map_key_t key1, map_key_t key2){
    if (key1.key_base == NULL || key2.key_base == NULL)
    {
        return false;
    }

    if (key1.key_len != key2.key_len)
    {
        return false;
    }

    if (memcmp(key1.key_base, key2.key_base, key1.key_len) == 0)
    {
        return true;
    }


    return false;
}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if (destroy_function == NULL || hash_function == NULL)
    {
        errno = EINVAL;
        return NULL;
    }

    if (capacity <= 0)
    {
        errno = EINVAL;
        return NULL;
    }

    hashmap_t* hashmap = Calloc(1, sizeof(hashmap_t));
    if ( hashmap == NULL)
    {
        return NULL;
    }

    hashmap -> capacity = capacity;
    hashmap -> hash_function = hash_function;
    hashmap -> destroy_function = destroy_function;

    map_node_t* nodes = Calloc(capacity, sizeof(map_node_t));

    if (nodes == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < capacity; ++i)
    {
        nodes[i].key.key_base = NULL;
        nodes[i].key.key_len = 0;
        nodes[i].val.val_base = NULL;
        nodes[i].val.val_len = 0;
        nodes[i].tombstone = true; // is tombstone

        // since i use calloc, everything after the address that nodes pointing to is 0;
    }

    hashmap -> nodes = nodes;
    hashmap->invalid = false;
    int p_w_ret = pthread_mutex_init(&hashmap->write_lock, NULL);
    if (p_w_ret != 0)
    {
        return NULL;
    }

    int p_f_ret = pthread_mutex_init(&hashmap->fields_lock, NULL);
    if (p_f_ret != 0)
    {
        return NULL;
    }

    return hashmap;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }
    if (key.key_base == NULL || key.key_len == 0)
    {
        errno = EINVAL;
        return false;
    }
    if (val.val_base == NULL || val.val_len == 0)
    {
        errno = EINVAL;
        return false;
    }


    pthread_mutex_lock(&self-> fields_lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self-> fields_lock);


    if (invalid == true)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self-> write_lock);
    int index = get_index(self, key);
    map_node_t* nodes = self -> nodes;
    int capacity = self ->capacity;

    bool key_exist = false;
    bool success = true;

    for (int i = 0; i < capacity; ++i)
    {

        int real_i = (index + i) % capacity;
        if (key_compare(key, nodes[real_i].key) == true && nodes[real_i].tombstone == false)
        {
            nodes[real_i].val = val;
            //pthread_mutex_unlock(&self-> write_lock);
            //return true;
            success = true;
            key_exist = true;
        }
    }

    //int i;
    //pthread_mutex_unlock(&self-> write_lock);

    //pthread_mutex_lock(&self-> write_lock);


    if(key_exist == false){
        for (int i = 0; i <= capacity; ++i)
        {
            if (i == capacity)
            { //full
                if (force == true)
                {

                    self->destroy_function(nodes[index].key, nodes[index].val);
                    nodes[index].key = key;
                    nodes[index].val = val;
                    nodes[index].tombstone = false;

                    break;
                }else{
                    errno = ENOMEM;
                    success = false;
                    break;
                }
            }

            int real_i = (i+index)%capacity;

            if (nodes[real_i].tombstone == true)
            {
                nodes[real_i].key = key;
                nodes[real_i].val = val;
                nodes[real_i].tombstone = false;

                //pthread_mutex_lock(&self->fields_lock);
                self->size++;
                //pthread_mutex_unlock(&self->fields_lock);

                break;
            }
        }
    }

    pthread_mutex_unlock(&self-> write_lock);



    return success;
}

map_val_t get(hashmap_t *self, map_key_t key) {

    if (self == NULL)
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    if (key.key_base == NULL || key.key_len == 0)
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    pthread_mutex_lock(&self-> fields_lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self-> fields_lock);

    if (invalid == true)
    {
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }

    pthread_mutex_lock(&self -> fields_lock);
    self-> num_readers ++;
    if (self -> num_readers == 1)
    {
        pthread_mutex_lock(&self->write_lock);
    }

    pthread_mutex_unlock(&self -> fields_lock);


    int capacity = self -> capacity;
    int index = get_index(self, key);
    map_node_t* nodes = self -> nodes;
    map_val_t ret_val_temp = MAP_VAL(NULL, 0);


    for (int i = 0; i < capacity; ++i)
    {

        int real_i = (i+index)%capacity;
        if (key_compare(key, nodes[real_i].key) == true && nodes[real_i].tombstone == false)
        {

            //pthread_mutex_lock(&self->fields_lock);
            //self-> num_readers ++;
            //pthread_mutex_unlock(&self->fields_lock);

            ret_val_temp = MAP_VAL(nodes[real_i].val.val_base, nodes[real_i].val.val_len);
            break;
        }

    }

    pthread_mutex_lock(&self->fields_lock);
    self -> num_readers --;
    if (self -> num_readers == 0)
    {
        pthread_mutex_unlock(&self-> write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);

    //pthread_mutex_unlock(&self-> write_lock);

    //return MAP_VAL(NULL, 0);

    return ret_val_temp;
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    if (self == NULL)
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    if (key.key_base == NULL || key.key_len == 0)
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    pthread_mutex_lock(&self-> fields_lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self-> fields_lock);

    if (invalid == true)
    {
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }


    pthread_mutex_lock(&self-> write_lock);
    int index = get_index(self, key);
    int capacity = self -> capacity;
    map_node_t* nodes = self -> nodes;
    map_node_t ret_node_temp = MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);

    for (int i = 0; i < capacity; ++i)
    {
        int real_i = (i+index)%capacity;
        if (key_compare(key, nodes[real_i].key) == true)
        {
            //pthread_mutex_lock(&self->fields_lock);
            nodes[real_i].tombstone = true;
            self -> size --;
            //pthread_mutex_unlock(&self->fields_lock);

            ret_node_temp = MAP_NODE(nodes[real_i].key, nodes[real_i].val, true);
            break;
        }
    }

    pthread_mutex_unlock(&self-> write_lock);

    //return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    return ret_node_temp;
}

bool clear_map(hashmap_t *self) {
    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self-> fields_lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self-> fields_lock);


    if (invalid == true)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self-> write_lock);
    int capacity = self -> capacity;
    //int size = self -> size;
    map_node_t* nodes = self -> nodes;

    for (int i = 0; i < capacity; ++i)
    {
        if (nodes[i].tombstone == false)
        {
            self -> destroy_function(nodes[i].key, nodes[i].val);

            //pthread_mutex_lock(&self->fields_lock);
            nodes[i].tombstone = true;
            self -> size --;
            //pthread_mutex_unlock(&self->fields_lock);
        }

    }


    pthread_mutex_unlock(&self-> write_lock);

	return true;
}

bool invalidate_map(hashmap_t *self) {
    debug("invalidate_map_start.");
    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self-> fields_lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self-> fields_lock);

    if (invalid == true)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self-> write_lock);
    int capacity = self -> capacity;
    map_node_t* nodes = self -> nodes;

    for (int i = 0; i < capacity; ++i)
    {
        if (nodes[i].tombstone == false)
        {
            self -> destroy_function(nodes[i].key, nodes[i].val);

            //pthread_mutex_lock(&self->fields_lock);
            nodes[i].tombstone = true;
            self -> size --;
            //pthread_mutex_unlock(&self->fields_lock);
        }
    }

    Free(nodes);
    //pthread_mutex_lock(&self->fields_lock);
    self->invalid = true;
    //pthread_mutex_unlock(&self->fields_lock);



    pthread_mutex_unlock(&self-> write_lock);
    debug("invalidate_map_end.");

    return true;
}



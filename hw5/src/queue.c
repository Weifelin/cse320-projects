#include "queue.h"
#include "csapp.h"
#include "debug.h"
//#include "hw5.h"

queue_t *create_queue(void) {

    debug("create_queue");

    queue_t* queue = Calloc(1,sizeof(queue_t));
    if (queue == NULL)
    {
        return NULL;
    }

    queue -> front = NULL;
    queue -> rear = NULL;
    //queue -> invalid is false(0) because of calloc. no need to initialzie it.
    int s_ret = Sem_init(&queue->items, 0, 0); // there are 0 items in the beginning.
    if (s_ret == -1)
    {
        return NULL;
    }
    int p_ret = pthread_mutex_init(&queue->lock, NULL);
    if (p_ret  != 0)
    {
        return NULL;
    }

    return queue;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {

    debug("invalidate_queue_start");

    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    if (destroy_function == NULL)
    {
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self->lock);

    if (invalid == true)
    {
        errno = EINVAL;
        return false;
    }
    //pthread_mutex_unlock(&self->lock);


    pthread_mutex_lock(&self->lock);

    queue_node_t* current_node = self -> front;
    queue_node_t* temp = current_node;
    while(current_node != NULL){
        temp = current_node->next;
        destroy_function(current_node->item);
        Free(current_node);
        current_node = temp;
    }

    self -> invalid = true;


    pthread_mutex_unlock(&self->lock);

    debug("invalidate_queue_ends");
    return true;
}

bool enqueue(queue_t *self, void *item) {

    if (self == NULL)
    {
        errno = EINVAL;
        return false;
    }

    if (item == NULL)
    {
        errno = EINVAL;
        return false;
    }


    pthread_mutex_lock(&self->lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self->lock);

    if (invalid == true)
    {
        errno = EINVAL;
        return false;
    }
    //pthread_mutex_unlock(&self->lock);



    pthread_mutex_lock(&self->lock);
    queue_node_t* node = Calloc(1, sizeof(queue_node_t));
    if (node == NULL)
    {
        exit(EXIT_FAILURE);
    }

    node -> item = item;
    node -> next = NULL;
    int items = 0;
    int sem_ret = sem_getvalue(&self->items, &items);
    pthread_mutex_unlock(&self->lock);

    if (sem_ret == -1)
    {
        printf("sem_getvalue error, exit.\n");
        return false;
    }


    pthread_mutex_lock(&self->lock);

    if (items == 0)
    {
        self->front = node;
        self->rear = node;

    }else{
        self->rear->next = node;
        self->rear = node;
    }

    pthread_mutex_unlock(&self->lock);

    V(&self->items); //update items by adding 1 to it.



    return true;
}

void *dequeue(queue_t *self) {
    if (self == NULL)
    {
        errno = EINVAL;
        return NULL;
    }


    pthread_mutex_lock(&self->lock);
    bool invalid = self -> invalid;
    pthread_mutex_unlock(&self->lock);



    if (invalid == true)
    {
        errno = EINVAL;
        return NULL;
    }


    P(&self->items);
    debug("after P");

    pthread_mutex_lock(&self->lock);
    debug("after lock");

    void* ret = NULL;
    if (self->front != NULL)
    {
        queue_node_t* new_front = self->front->next;
        queue_node_t* old_front = self->front;
        ret = old_front -> item;
        self -> front = new_front;
        Free(old_front);
    }


    pthread_mutex_unlock(&self->lock);

    return ret;
}

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include "sfmm2.h"
#include <string.h>
#include <errno.h>
//#include "lib/sfutil.o"
//#include "sfutil.o"

#define SPLINTER 32
#define SF_ALIGN 16
#define SF_ROW 8 // 2 bytes/ WORD, 4 WORDS/ROW


/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};


int sf_errno = 0;

//sf_free_headers[FREE_LIST_COUNT];

sf_header* get_header_pointer_from_fp(sf_footer* fp){
    size_t blocksize = fp -> block_size << 4;
    sf_header* header_adrs = (sf_header*)((void*)fp - blocksize + 8); //8 is the header size. 8 bytes.
    return header_adrs;
}
sf_footer* get_footer_pointer_from_hp(sf_header* hp){
    size_t blocksize = hp -> block_size << 4;
    sf_footer* footer_adrs = (sf_footer*)((void*)hp + blocksize - 8);
    return footer_adrs;
}

int add_to_free_list(sf_free_header* to_be_added){
    int ret = -1;
    size_t blocksize = to_be_added-> header.block_size << 4;

    for (int i = 0; i < FREE_LIST_COUNT; ++i){
        uint16_t min_l = seg_free_list[i].min;
        uint16_t max_l = seg_free_list[i].max;



        if (blocksize >= min_l && blocksize <= max_l)
        {
            sf_free_header* current_head = seg_free_list[i].head;

            if (current_head != NULL)
            {
                current_head -> prev = to_be_added;
                to_be_added -> next = current_head;
                to_be_added -> prev = NULL;
                seg_free_list[i].head = to_be_added;
            }else{
                to_be_added -> next = NULL;
                to_be_added -> prev = NULL;
                seg_free_list[i].head = to_be_added;
            }

            return 1;
        }
    }

    return ret;
}

int remove_from_free_list(sf_free_header* to_be_removed, free_list* list){
    int ret = -1;
    sf_free_header* pre_temp_p = to_be_removed -> prev;
    sf_free_header* next_temp_p = to_be_removed -> next;



    if ((pre_temp_p == NULL) && (next_temp_p== NULL))
    {
        list -> head = NULL;
        to_be_removed -> prev = NULL;
        to_be_removed -> next = NULL;
        return 1;
    }

    if ((pre_temp_p == NULL) && (next_temp_p != NULL))
    {
        next_temp_p -> prev = NULL;
        list -> head = next_temp_p;
        to_be_removed -> prev = NULL;
        to_be_removed -> next = NULL;
        return 1;
    }

    if ((pre_temp_p != NULL) && (next_temp_p == NULL))
    {
        pre_temp_p -> next = NULL;
        to_be_removed -> prev = NULL;
        to_be_removed -> next = NULL;
        return 1;
    }

    if ((pre_temp_p != NULL) && (next_temp_p != NULL))
    {
        pre_temp_p -> next = next_temp_p;
        next_temp_p -> prev = pre_temp_p;
        to_be_removed -> prev = NULL;
        to_be_removed -> next = NULL;
        return 1;
    }

    return ret;

}

sf_free_header* mem_init(void* page_start){
    /*if (sf_sbrk() == (void*)(-1))
        {
            return NULL;
        }*/

        sf_header* header_p;
        sf_footer* footer_p;


        ///char* bp = get_heap_start();
        //header_p = (sf_header*) get_heap_start();
        header_p = (sf_header*) page_start;

        int sfbloc_sz = PAGE_SZ;

        int zeros = 0;

        //*header_p = 0;
        memcpy(header_p, &zeros, 8);





        header_p -> block_size = sfbloc_sz >> 4;



        footer_p = get_heap_end()-8;

        //printf("header_p: %li footer_p: %li\n", (long) header_p, (long)footer_p);

        memcpy(footer_p, &zeros, 8);

        //*footer_p = 0;
        footer_p -> block_size = sfbloc_sz >> 4;

        for (int i = 0; i < FREE_LIST_COUNT; ++i)
        {
            uint16_t min_l = seg_free_list[i].min;
            uint16_t max_l = seg_free_list[i].max;
            if (sfbloc_sz >= min_l && sfbloc_sz <= max_l)
            {
                //sf_free_header headers;
                sf_free_header* headers = (sf_free_header*)header_p;
                //headers.header = *header_p;
                headers ->next =  NULL;
                headers ->prev = NULL;

                /*seg_free_list[i].head -> header = header_p; //pointer.
                seg_free_list[i].head -> next = NULL;
                seg_free_list[i].head -> prev = NULL;*/
                //seg_free_list[i].head = &headers;
                //seg_free_list[i].head = headers;


                if (seg_free_list[i].head != NULL)
                {
                    add_to_free_list(headers);
                }else{
                    seg_free_list[i].head = headers;
                }

                //break;
                return seg_free_list[i].head;
                //return headers;
            }

        }

        return NULL;
}

sf_free_header* search_in_list(size_t blocksize, sf_free_header* listhead){
    sf_free_header* current = listhead;
    size_t current_block_size = 0;

    while(current != NULL){
        current_block_size = (size_t) current -> header.block_size << 4;
        if (current_block_size >= blocksize)
        {
            return current;
        }

        current_block_size=0;
        current = current -> next;
    }

    return NULL;
}

sf_free_header* search_head_in_list(sf_header* toCoalesce, sf_free_header* listhead){
    sf_free_header* current = listhead;

    while(current != NULL){
        if ( &(current-> header) == toCoalesce)
        {
            return current;
        }

        current = current -> next;
    }

    return NULL;

}

void *sf_malloc(size_t size) {
    //checking if the size is valid.
    if ((size == 0) || (size > 4*PAGE_SZ))
    {
        sf_errno = EINVAL;
        return NULL;
    }

    void* page_start;
    size_t adjusted_sz = size;
    char paded = 0;

    /*sf_header alloc_h = 0;
    sf_footer alloc_f = 0;*/

    //sf_header* header_p = NULL;
    //sf_footer* footer_p = NULL;

    void* payload_start = NULL;

    sf_free_header* sf_free_header_temp;




    // setting big gaint free block.

    if ((seg_free_list[0].head == NULL) && (seg_free_list[1].head == NULL) && (seg_free_list[2].head == NULL) && (seg_free_list[3].head == NULL) )
    {
        page_start = sf_sbrk();

        if (page_start == (void*)(-1))
        {
            return NULL;
        }

        //printf("%s  page_start:%li  get_heap_start:%li\n", "Line 235", (long)page_start, (long)get_heap_start());

        sf_free_header_temp =  mem_init(page_start);


        if (sf_free_header_temp == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
    }

    ///////////////////////////////////////////
    if (size < SF_ALIGN)
    {
        adjusted_sz = SF_ALIGN;
        paded = 1;
    }else if ((size%SF_ALIGN) == 0)
    {
        adjusted_sz = size;
        paded = 0;
    }else{
        adjusted_sz = (size/SF_ALIGN + 1)*SF_ALIGN;
        paded = 1;
    }

    //search for right free block.

    sf_free_header* sfh = NULL;
    size_t sfbloc_sz = 16 + adjusted_sz; // used in header, int is 4 bytes -> 32 bits. 16 is the total bytes of header and footer.
    //printf("sfbloc_sz: %d\n", (int)sfbloc_sz);

    for (int i = 0; i < FREE_LIST_COUNT; ++i)
    {
        free_list list = seg_free_list[i];
        sfh = search_in_list(sfbloc_sz, list.head);


        if (sfh != NULL)
        {
            //sf_snapshot();
            //printf("sfh block_size: %i\n", sfh-> header.block_size <<4);
            size_t sfh_blocksz = sfh -> header.block_size << 4;
            //size_t reminder_blck_sz = sfh_blocksz - sfbloc_sz;
            for (int i = 0; i < FREE_LIST_COUNT; ++i)
            {
                uint16_t min_l = seg_free_list[i].min;
                uint16_t max_l = seg_free_list[i].max;
                if (sfh_blocksz >= min_l && sfh_blocksz <= max_l)
                {
                    remove_from_free_list(sfh, &seg_free_list[i]);
                }
            }

            break;
        }


    }

    while (sfh == NULL)
    {
        /* When there is no suitable blocks.
           call sf_brk and mem_init, and try to coalsce  */

        page_start = sf_sbrk();

        if (page_start == (void*)(-1))
        {
            return NULL;
        }

        sf_free_header_temp =  mem_init(page_start);
        if (sf_free_header_temp == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }

        //try to coalesce with previous block.

        //sf_snapshot();
        sf_header* h_p = &(sf_free_header_temp -> header);
        sf_footer* fp = get_footer_pointer_from_hp(h_p);
        sf_footer* pre_fp = (sf_footer*)((void*)h_p - 8);
        sf_header* pre_hp = get_header_pointer_from_fp(pre_fp);

        sf_free_header* toCoalesce;
        size_t blck_sz = pre_hp-> block_size <<4;
        //printf("allocated: %i, %i\n", pre_hp->allocated, pre_fp->allocated);

        if(!(pre_fp->allocated)){



            for (int i = 0; i < FREE_LIST_COUNT; ++i)
            {
                uint16_t min_l = seg_free_list[i].min;
                uint16_t max_l = seg_free_list[i].max;

                if (blck_sz >= min_l && blck_sz <= max_l)
                {
                    toCoalesce = search_head_in_list(pre_hp, seg_free_list[i].head);
                    //printf("toCoalesce: %i, i: %i\n", toCoalesce->header.block_size <<4, i);
                    if (toCoalesce == NULL)
                    {
                        printf("%s\n", "Cannot Find this free header in free_list");
                        abort();
                    }

                    //remove toCoalesce from freeList.
                    int is_removed = remove_from_free_list(toCoalesce, &seg_free_list[i]);
                    if (is_removed == -1)
                    {
                        printf("%s\n", "Not removed sucessfully.");
                    }


                    break;
                }
            }



            sf_header* new_header_p = pre_hp;
            sf_footer* new_footer_p = fp;

            //size_t current_blck_sz =  h_p -> block_size <<4;
            //size_t new_blck_sz = blck_sz + current_blck_sz;
            size_t new_blck_sz = (h_p-> block_size << 4) + (pre_hp->block_size <<4);
            int zeros = 0;

            memcpy(new_header_p, &zeros, 8);
            memcpy(new_footer_p, &zeros, 8);

            new_header_p -> block_size = new_blck_sz >> 4;
            new_footer_p -> block_size = new_blck_sz >> 4;

            sf_free_header* to_be_added = (sf_free_header*) new_header_p;
            to_be_added -> next = NULL;
            to_be_added -> prev = NULL;

            add_to_free_list(to_be_added);



            /*if (new_blck_sz >= sfbloc_sz)
            {
                sfh = to_be_added;
            }*/
        }

        for (int i = 0; i < FREE_LIST_COUNT; ++i)
        {
            free_list list = seg_free_list[i];
            sfh = search_in_list(sfbloc_sz, list.head);

            if (sfh != NULL)
            {

                size_t sfh_blocksz = sfh -> header.block_size << 4;
            //size_t reminder_blck_sz = sfh_blocksz - sfbloc_sz;
                for (int i = 0; i < FREE_LIST_COUNT; ++i)
                {
                    uint16_t min_l = seg_free_list[i].min;
                    uint16_t max_l = seg_free_list[i].max;
                    if (sfh_blocksz >= min_l && sfh_blocksz <= max_l)
                    {
                        remove_from_free_list(sfh, &seg_free_list[i]);
                    }
                }
                break;
            }
        }
    }

    //sf_snapshot();

    /*Suppose now sfh contains the right block.*/
    // splint
    size_t sfh_blocksz = sfh -> header.block_size << 4;
    size_t reminder_blck_sz = sfh_blocksz - sfbloc_sz;
    for (int i = 0; i < FREE_LIST_COUNT; ++i)
    {
        uint16_t min_l = seg_free_list[i].min;
        uint16_t max_l = seg_free_list[i].max;
        if (sfh_blocksz >= min_l && sfh_blocksz <= max_l)
        {
            remove_from_free_list(sfh, &seg_free_list[i]);
        }
    }

    //printf("%s\n", "Line 400");


    if (reminder_blck_sz >= 32)
    {
        /*split the blokc into two block. One is allocated and one is free.*/
        size_t new_block_size = sfbloc_sz;
        sf_header* new_allocated_header = &(sfh -> header);
        sf_footer* new_allocated_footer = (sf_footer*)((void*)new_allocated_header+new_block_size-8);

        int zeros = 0;
        memcpy(new_allocated_header, &zeros, 8);
        memcpy(new_allocated_footer, &zeros, 8);

        new_allocated_header -> block_size = new_block_size >> 4;
        new_allocated_footer -> block_size = new_block_size >> 4;
        new_allocated_footer -> requested_size = size;
        if (paded == 1)
        {
            new_allocated_header -> padded = 1;
            new_allocated_footer -> padded = 1;
        }

        new_allocated_header -> allocated = 1;
        new_allocated_footer -> allocated = 1;



        payload_start = (void*) new_allocated_header + 8;

        //dealing with new free block.
        sf_header* new_free_header = (sf_header*)((void*)new_allocated_header + new_block_size);
        sf_footer* new_free_footer = (sf_footer*) ((void*)new_free_header + reminder_blck_sz - 8);
        memcpy(new_free_header, &zeros, 8);
        memcpy(new_free_footer, &zeros, 8);

        //printf("reminder_blck_sz: %li\n", reminder_blck_sz);

        new_free_header -> block_size = reminder_blck_sz >> 4;
        new_free_footer -> block_size = reminder_blck_sz >> 4;

        sf_free_header* new_free_blck = (sf_free_header*) new_free_header;
        new_free_blck ->next = NULL;
        new_free_blck -> prev = NULL;

        int add_b = add_to_free_list(new_free_blck);
        //sf_snapshot();



        if (add_b == -1)
        {
            printf("%s\n", "Unable to add new free block to free list.");
            abort();
        }

        return payload_start;
    }

    //printf("%s\n", "Line 448");
    sf_header* allcted_header = &(sfh -> header);
    sf_footer* allcted_footer = (sf_footer*) ((void*)allcted_header + sfh_blocksz -8);

    int zeros_f = 0;
    memcpy(allcted_header, &zeros_f, 8);
    memcpy(allcted_footer, &zeros_f, 8);

    allcted_header -> block_size = sfh_blocksz >> 4;
    allcted_footer -> block_size = sfh_blocksz >> 4;
    allcted_footer -> requested_size = size;

        if (paded == 1)
        {
            allcted_header -> padded = 1;
            allcted_footer -> padded = 1;
        }

        allcted_footer -> allocated = 1;
        allcted_header -> allocated = 1;


    payload_start = (void*) ((void*)allcted_header + 8);


    return payload_start;
}


void sf_free(void *ptr) {
    if (ptr == NULL)
    {
        abort();
    }

    void* heap_start_p = get_heap_start();
    void* heap_end_p = get_heap_end();
    void* footer_adrs_limit = heap_end_p -8;

    sf_header* ptr_header = (sf_header*) (ptr-8);

    size_t blocksize = ptr_header->block_size << 4;

    /*sf_footer* ptr_footer = (sf_footer*)ptr_header;
    ptr_footer = ptr_footer + blocksize;
    ptr_footer = ptr_footer -8;*/

    sf_footer* ptr_footer = (sf_footer*)((void*)ptr_header + blocksize - 8);

    size_t requestedsize = ptr_footer -> requested_size;

    size_t non_padded_blck_sz = requestedsize + 16;


    if ( ((void*)ptr_header < heap_start_p) || ((void*) ptr_footer > footer_adrs_limit))
    {
        abort();
    }
    if ((ptr_header-> block_size<<4) != (ptr_footer -> block_size <<4))
    {
        abort();
    }
    if((ptr_header -> padded) != (ptr_footer -> padded)){
        abort();
    }

    if((ptr_header -> allocated) != (ptr_footer -> allocated)){
        abort();
    }

    if ((ptr_header -> allocated == 0) || (ptr_footer -> allocated == 0) )
    {
       abort();
    }

    if ((non_padded_blck_sz != blocksize) && (ptr_header -> padded == 0)) //padded while padded = 0
    {
        abort();
    }

    if ((non_padded_blck_sz == blocksize) && (ptr_header -> padded == 1)) // non_padded while padded == 1
    {
        abort();
    }

    if (requestedsize > blocksize)
    {
        abort();
    }

    if (blocksize % 16 != 0)
    {
        abort();
    }

    //ptr now should be valid.
    int zero = 0;


    sf_header* next_header = (sf_header*) ((void*)ptr_footer+8);
    size_t next_blck_sz = next_header -> block_size << 4;
    sf_footer* next_footer = (sf_footer*) ((void*)next_header + next_blck_sz - 8);

    sf_free_header* list_header_temp = NULL;

    if (next_header -> allocated == 0) // this block is free-> can be coalesced.
    {
        for (int i = 0; i < FREE_LIST_COUNT; ++i)
        {
            uint16_t min_l = seg_free_list[i].min;
            uint16_t max_l = seg_free_list[i].max;



            if (next_blck_sz >= min_l && next_blck_sz <= max_l)
            {
                //printf("next_blck_sz: %li, min_l: %i, max_l: %i\n", next_blck_sz, min_l, max_l);

                //sf_snapshot();

                list_header_temp = search_head_in_list(next_header, seg_free_list[i].head);
                if (list_header_temp == NULL)
                {
                    printf("%s\n", "Cannot find the free block in freelist. Line: 590");
                }
                //remove from free list.
                //sf_snapshot();

                int fg = remove_from_free_list(list_header_temp, &seg_free_list[i]);

                //sf_snapshot();

                if (fg == -1)
                {
                    printf("%s\n", "Failed to remove from free list");
                }
                break;
            }
        }

        // the next free block has been removed from free list.

        size_t new_free_sz = blocksize + next_blck_sz;
        sf_header* new_free_header = ptr_header;
        sf_footer* new_free_footer = next_footer;


        memcpy(new_free_header, &zero, 8);
        memcpy(new_free_footer, &zero, 8);

        memcpy(ptr_footer, &zero, 8);
        memcpy(next_header, &zero, 8);

        new_free_header -> block_size = new_free_sz >> 4;
        new_free_footer -> block_size = new_free_sz >> 4;

        list_header_temp = (sf_free_header*) new_free_header;
        list_header_temp -> next = NULL;
        list_header_temp -> prev = NULL;

        int added = add_to_free_list(list_header_temp);
        if (added == -1)
        {
            printf("%s\n", "Faild to add coalesced block to free list");
        }

        return;
    }


    memcpy(ptr_header, &zero, 8);
    memcpy(ptr_footer, &zero, 8);
    ptr_header -> block_size = blocksize >> 4;
    ptr_footer -> block_size = blocksize >> 4;

    list_header_temp = (sf_free_header*)ptr_header;
    list_header_temp -> next = NULL;
    list_header_temp -> prev = NULL;


    int add_flg = add_to_free_list(list_header_temp);
    if (add_flg == -1)
    {
        printf("%s\n", "Faild to add coalesced block to free list");
    }

   /* ptr_header -> allocated = 0;
    ptr_footer -> allocated = 0;*/

    return;
}



//////

void *sf_realloc(void *ptr, size_t size) {
    if (ptr == NULL)
    {
        abort();
    }

    void* heap_start_p = get_heap_start();
    void* heap_end_p = get_heap_end();
    void* footer_adrs_limit = heap_end_p -8;

    sf_header* ptr_header = (sf_header*) (ptr-8);

    size_t blocksize = ptr_header->block_size << 4;

    size_t current_payload_sz = blocksize - 16;

    sf_footer* ptr_footer = (sf_footer*)((void*)ptr_header + blocksize - 8);

    size_t requestedsize = ptr_footer -> requested_size;

    size_t non_padded_blck_sz = requestedsize + 16;


    if ( ((void*)ptr_header < heap_start_p) || ((void*) ptr_footer > footer_adrs_limit))
    {
        abort();
    }
    if ((ptr_header-> block_size<<4) != (ptr_footer -> block_size <<4))
    {
        abort();
    }
    if((ptr_header -> padded) != (ptr_footer -> padded)){
        abort();
    }

    if((ptr_header -> allocated) != (ptr_footer -> allocated)){
        abort();
    }

    if ((ptr_header -> allocated == 0) || (ptr_footer -> allocated == 0) )
    {
       abort();
    }

    if ((non_padded_blck_sz != blocksize) && (ptr_header -> padded == 0)) //padded while padded = 0
    {
        abort();
    }

    if ((non_padded_blck_sz == blocksize) && (ptr_header -> padded == 1)) // non_padded while padded == 1
    {
        abort();
    }

    if (requestedsize > blocksize)
    {
        abort();
    }

    if (blocksize % 16 != 0)
    {
        abort();
    }

    if (size == 0)
    {
        sf_free(ptr);
        return NULL;
    }

    int zero =0;


//adjust size
    size_t adjusted_sz = size;
    char is_adjusted = 0;

    if (size < SF_ALIGN)
    {
        adjusted_sz = SF_ALIGN;
        is_adjusted = 1;
    }else if ((size%SF_ALIGN) == 0)
    {
        adjusted_sz = size;
        is_adjusted = 0;
    }else{
        adjusted_sz = (size/SF_ALIGN + 1)*SF_ALIGN;
        is_adjusted = 1;
    }

// reallocating to larger size.
    void* return_payload = NULL;

    if (adjusted_sz == current_payload_sz)
    {
        ptr_footer -> requested_size = size;


        if (is_adjusted == 1)
        {
            ptr_header -> padded = 1;
            ptr_footer -> padded = 1;
        }

        if (is_adjusted == 0)
        {
            ptr_header -> padded = 0;
            ptr_footer -> padded = 0;
        }


        return_payload = ptr;
        return return_payload;
    }

    //reallocating to larger size.

    if (adjusted_sz > current_payload_sz)
    {
        void* payload_temp = sf_malloc(adjusted_sz);
        //sf_header* new_hp = (sf_header*)(payload_temp - 8);
        //size_t l_blck_sz = new_hp -> block_size << 4;

        //ptr_header -> block_size = l_blck_sz >> 4;

        memcpy(payload_temp, ptr, requestedsize);
        sf_free(ptr);

        return_payload = payload_temp;
        return return_payload;
    }
    // to smaller size.
    if (adjusted_sz < current_payload_sz)
    {
        if((current_payload_sz - adjusted_sz) < SPLINTER){
            ptr_footer -> requested_size = size;
            ptr_footer -> padded = 1;
            ptr_header -> padded = 1;

            return_payload = ptr;
            return return_payload;
        }

        if((current_payload_sz - adjusted_sz) >= SPLINTER ){
            size_t new_small_allocated_blck_sz = adjusted_sz + 16;
            sf_header* new_small_allocated_hp = ptr_header;
            sf_footer* new_small_allocated_fp = (sf_footer*)((void*)ptr_header + new_small_allocated_blck_sz -8);

            sf_header* new_free_header = (sf_header*)((void*)new_small_allocated_fp + 8);
            sf_footer* new_free_footer = ptr_footer;
            size_t new_free_blck_sz = blocksize - new_small_allocated_blck_sz;

            //setting new header/footer of allocated block.
            new_small_allocated_hp-> block_size = new_small_allocated_blck_sz >> 4;
            memcpy(new_small_allocated_fp, ptr_footer, 8);
            new_small_allocated_fp-> block_size = new_small_allocated_blck_sz >> 4;
            new_small_allocated_fp-> requested_size = size;
            if (is_adjusted == 1)
            {
                new_small_allocated_hp -> padded = 1;
                new_small_allocated_fp -> padded = 1;
            }
            if (is_adjusted == 0)
            {
                new_small_allocated_fp -> padded = 0;
                new_small_allocated_hp -> padded = 0;
            }

            //setting new free block's header and footer.
            memcpy(new_free_header, &zero, 8);
            memcpy(new_free_footer, &zero, 8);
            new_free_header->block_size = new_free_blck_sz >> 4;
            new_free_footer->block_size = new_free_blck_sz >> 4;
            new_free_footer -> allocated = 1;
            new_free_header -> allocated = 1;

            new_free_footer -> requested_size = new_free_blck_sz - 16; //in order to make sf_free() work for this block.
            void* to_be_freed = (void*)new_free_header+8;
            sf_free(to_be_freed);

            return_payload = ptr;
            return return_payload;
        }
    }





    return NULL;
}



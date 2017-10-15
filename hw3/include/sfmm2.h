
#ifndef SFMM2_H
#define SFMM2_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "sfmm.h"



sf_free_header* mem_init(void* page_start);// call sf_brk and init the new page.

sf_free_header* search_in_list(size_t blocksize, sf_free_header* listhead);


sf_free_header* search_head_in_list(sf_header* toCoalesce, sf_free_header* listhead);

sf_header* get_header_pointer_from_fp(sf_footer* fp);

sf_footer* get_footer_pointer_from_hp(sf_header* hp);

int add_to_free_list(sf_free_header* to_be_added);

int remove_from_free_list(sf_free_header* to_be_removed, free_list* list);


#endif
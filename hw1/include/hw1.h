#ifndef HW_H
#define HW_H

#include "const.h"
//#include <stdio.h>

#endif

#define TRUE 1
#define FALSE 0

int string_eql(char* str1, char*str2);
int string_to_int(char* str);
int has_repeated_char(char* key);// return 1(TRUE) if key has repeated char and 0(FLASE) if it doesn't.



//return 1(TRUE) if all letters in key are in alphabet. 0 otherwise.
int isLegal_in_alphabet(char* key, const char* alphabet);

//return EXIT_SUCCESS if no errors. EXITFAILURE otherwise.
unsigned int cipher_pe(unsigned short mode);
unsigned int cipher_pd(unsigned short mode);
unsigned int cipher_fe();
unsigned int cipher_fd();

unsigned int cal_Index(unsigned int row_i, unsigned int col_i, unsigned int col);
int is_char_in_key(char chr);
int get_index_in_table(char chr);

unsigned int char_to_Index(char chr);
int get_morse_table_index(char chr);

//add string to buffer. and return index.
int add_to_buffer(char* buffer, char* morsed, int buffer_index);

//return the number of elements in buffer.
int buffer_size(char* buffer);

//return the index after the last element.
int shift_buffer_back3(char* buffer);

int shift_buffer_back1(char* buffer);

int get_frac_table_index(char* fm_buff);

int get_fm_key_index(char letter);

//return the index after the last element.
int shift_buffer_backi(char* buffer, int amount);


int string_eql_p3(char* str1, char* str2);


int get_index_in_morse_table(char* morse_c_buff);





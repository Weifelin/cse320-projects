#include "hw1.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


int buffer_size(char* buffer){
    int count = 0;

    for (int i = 0; *(buffer + i) != '\0'; i++)
    {
        count++;
    }

    return count;

}

int string_eql(char* str1, char* str2){

    int chrc = 0;
    char char1 = *(str1+chrc);
    char char2 = *(str2+chrc);

    if (buffer_size(str1) != buffer_size(str2))
    {
        return 0;
    }

    while( char1 != '\0' && char2 != '\0'){
        if (char1 != char2)
        {
            return 0; //false
        }
        chrc ++;
        char1 = *(str1+chrc);
        char2 = *(str2+chrc);
    }

    return 1; //true
}





int string_to_int(char* str){
    int neg =  1;  // turn to -1 if chars has '-'.
    int str_index = 0;

    if (*(str+0) == '-')
    {
        neg = -1;
        str_index = 1;
    }

    int ans = 0;

    char digit = *(str + str_index);

    while(digit != '\0'){
        ans = ans*10 + (digit - '0');
        str_index++;
        digit = *(str + str_index);
    }

    ans = ans * neg;

    return ans;
}

//check if the key has repeated char. if key has, return 1(Ture), and 0(Flase) otherwise.
int has_repeated_char(char* key){

    int key_index = 0;
    char current_symbol = *(key+key_index);
    //printf("current_symbol: %c\n", current_symbol);
    //printf("Key: %s\n", key);


    for (key_index=0; *(key+key_index+1)!= '\0'; key_index++)
    {
        current_symbol = *(key+key_index);


        int index = key_index+1;

        while(*(key+index) != '\0'){
            //printf("current_symbol: %c\n", current_symbol);
            //printf("%c\n", *(key+index));

            if (current_symbol == *(key+index))
            {
                return 1;//true
            }
            index++;
        }
    }

    return 0;//false.
}


//return 1 if this char is in the alphabet. 0 otherwise.
int is_char_in_alphabet(char letter, const char* alphabet){
    int index = 0;
    while(*(alphabet+index)!= '\0'){
        if (letter == *(alphabet+index))
        {
            return 1;//TRUE
        }
        index++;
    }

    return 0;//FALSE
}
//return 1(TRUE) if all letters in key are in alphabet. 0 otherwise.
int isLegal_in_alphabet(char* key, const char* alphabet){
    int key_i = 0;
    while(*(key+key_i) != '\0'){
        if (is_char_in_alphabet(*(key+key_i), alphabet) == FALSE)
        {
            return 0; //FALSE - illegal
        }
        key_i++;
    }

    return 1; //TRUE
}



/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
unsigned short validargs(int argc, char **argv) {

    if (argc<2) //too few arguments.
    {
        return 0;
    }

    char* arg1 = *(argv+1);


    if (string_eql(arg1,"-h") == TRUE)
    {
        return 0x8000;
    }

    if(argc > 9){ //too many arguments
        return 0;
    }

    //Now, arg1 is not "-h". The minimal argc should be 3.eg: bin/hw1 -f -e.
    if (argc<3)
    {
        return 0;
    }


    char* arg2 = *(argv+2);
    if ((string_eql(arg1,"-p")==FALSE) && (string_eql(arg1,"-f")==FALSE))
    {

        //if arg1 isn't neither "-p" nor "-f". The arguments is invalid.
        return 0;
    }

    if ((string_eql(arg2,"-e")==FALSE) && (string_eql(arg2,"-d")==FALSE))
    {
        //if arg2 isn't neither "-e" nor "-d". The arguments is invalid.
        return 0;
    }


    //char* arg_k = "TEST_K";
    char* argKey = NULL;
    //char* arg_r = "TEST_R";
    int argRow = 10;
    //char* arg_c = "TEST_C";
    int argCol = 10;

    key = NULL;

    int argk_count = 0;
    int argr_count = 0;
    int argc_count = 0;


    //printf("%s\n", arg1);
    //printf("%s\n",arg2 );

    for(int index = 3; index<argc; index++){


        if (string_eql(*(argv+index),"-k") == TRUE)
        {

            argk_count++;

            if (argk_count>1){
                return 0;
            }

            //arg_k = *(argv+index);
            argKey = *(argv+index+1);



            /*do key checking here. Must have no repeated char.
              == if arg1 is "-p". each char must be in "polybius_alphabet" in "const.c".
              == if arg1 is "-f". each char must be in "fm_alphabet" in "const.c".
              **************************************
              "-p" and "-f" have different alphabets.
              **************************************
            */
            if (has_repeated_char(argKey) == TRUE)
            {
                return 0;
            }

            //printf("argk_count: %i\n", argk_count);
            //printf("Index: %i. Argument: %s\n",index, *(argv+index) );
            //printf("argKey: %s\n", argKey );

            //now check if all chars in key is in respective alphabet.
            if (string_eql(arg1,"-p") == TRUE)
            {
                if (isLegal_in_alphabet(argKey, polybius_alphabet) == FALSE)
                {
                    return 0;
                }
            }

            if (string_eql(arg1, "-f") == TRUE)
            {
                if(isLegal_in_alphabet(argKey, fm_alphabet) == FALSE){
                    return 0;
                }
            }

            key = argKey; //argKey is char* and key is char* also.

        }

        if(string_eql(*(argv+index),"-r") == TRUE){

            if (string_eql(arg1,"-f") == TRUE) // "-f" cannot carry "-r" and "-c".
            {
                return 0;
            }

            argr_count++;
            if (argr_count >1)
            {
                return 0;
            }

            //arg_r = *(argv+index);


            char* row_str = *(argv+index+1);//still a string;
            int row_temp = string_to_int(row_str);
            if (row_temp<9 || row_temp>15) //invalid.
            {
                return 0;
            }

            argRow = row_temp;

        }

        if (string_eql(*(argv+index),"-c")== TRUE)
        {

            if (string_eql(arg1,"-f") == TRUE) // "-f" cannot carry "-r" and "-c".
            {
                return 0;
            }

            argc_count++;
            if (argc_count>1)
            {
                return 0;
            }

            //arg_c = *(argv+index);

            char* col_str = *(argv+index+1); //still a string
            int col_temp = string_to_int(col_str);
            if (col_temp<9 || col_temp>15) //invalid.
            {
                return 0;
            }

            argCol = col_temp;
        }
    }

    unsigned short return_value = 0x0000;

    if (string_eql(arg1, "-h") == TRUE)
    {
        return_value = 0x8000;
        return return_value;
    }


    //printf("ARG1: %s\n", arg1);

    if (string_eql(arg1, "-f"))
    {
        return_value = return_value | 0x4000; //0x4000 = 0100 0000 0000 0000 in binary.
    } else if (string_eql(arg1, "-p"))
    {
        /* code */
        return_value = return_value & 0xBFFF; //0xBFFF = 1011 1111 1111 1111 in binary. This keeps other bits while sets 2nd bit to 0.
        unsigned short shift_row_value = 16; //"2^4=16" means shift to left 4 bits.

        //printf("shift_row_value:%i\n", shift_row_value);

        unsigned short row_value = argRow*shift_row_value;
        //printf("ROW: 0x%X\n", row_value);
        //clear the last 8 bits, making them 0;
        return_value = return_value & 0xFF00; //0xFF00 = 1111 1111 0000 0000 in binary. This AND keeps the first 8 bits and sets the least 8 bits to 0.
        return_value = return_value | row_value;

        //printf("ROW&REturn : 0x%X\n", return_value);

        return_value = return_value | argCol;

        //printf("ROW&COL: 0x%X\n", return_value);


    }

    if (string_eql(arg2,"-e"))
    {
        return_value = return_value & 0xDFFF; //0xDFFF = 1101 1111 1111 1111 in binary. This AND keeps other bits while set the 3rd bit to 0.

    }else if(string_eql(arg2, "-d")){
        return_value = return_value | 0x2000;//0x2000 = 0010 0000 0000 0000 in binary. This OR keeps other bits while sets third bit to 1.
    }

    //test
    // printf("%s\n", "Test:");
    // printf("%i\n", argc);
    // printf("%s\n", arg1);
    // printf("%s\n", arg2);
    // printf("%s %s\n", arg_k, argKey);
    // printf("%s %i\n", arg_c, argCol);
    // printf("%s %i\n", arg_r, argRow);
    // printf("0x%X\n", return_value);

    return return_value;



    //return 0x8000;
}




unsigned int cal_Index(unsigned int row_i, unsigned int col_i, unsigned int col){
    unsigned int index = row_i*col+col_i;
    return index;
}


int is_char_in_key(char chr){
    int index = 0;

    while(*(key+index)!= '\0'){
        if (chr == *(key+index))
        {
            return 1;//TRUE
        }
        index++;
    }

    return 0;//FALSE
}

int get_index_in_table(char chr){
    for (int i = 0; *(polybius_table+i) != '\0'; i++)
    {
        if (chr == *(polybius_table + i))
        {
            return i;
        }
    }

    return -1;
}

char int_to_char(int index){ // the index is row_i or col_i
    char output = '\0';

    if (index == 10)
    {
        output = 'A';
    }else if (index == 11)
    {
        output = 'B';
    }else if (index == 12)
    {
        output = 'C';
    }else if (index == 13)
    {
        output = 'D';
    }else if (index == 14)
    {
        output = 'E';
    }else if (index == 15){
        output = 'F';
    }else {
        output = index + '0';
    }

    return output;
}

unsigned int cipher_pe(unsigned short mode){


    // getting row
    unsigned int row_temp = mode & 0x00F0;//0x00F0 = 0000 0000 1111 0000. This keeps the 1111 part in mode.
    unsigned int row = row_temp/16; //shift back 4 bits. 2^4 = 16.

    // getting column

    unsigned int col = mode & 0x000F; //0x000f = 0000 0000 0000 1111. This keeps the leftmost 4 bits.

    unsigned int total_elements_in_table = row*col;
    //construct table.
    if (key != ((char*)NULL)){

        int i = 0;
        for (i = 0; *(key+i) != '\0'; i++)
        {
            *(polybius_table+i) = *(key+i);
        }

        for (int j = 0; *(polybius_alphabet+j)!='\0'; j++)
        {
            if (is_char_in_key(*(polybius_alphabet+j)) == FALSE)
            {
                *(polybius_table+i) = *(polybius_alphabet+j);
                i++;
            }
        }

        for (int k = i; k < total_elements_in_table; k++)
        {
            *(polybius_table+k) = '\0';
        }
    }else {

        int i=0;
        for (i = 0; i < total_elements_in_table; i++)
        {
            if (*(polybius_alphabet+i) != '\0')
            {
                *(polybius_table+i) = *(polybius_alphabet+i);
            }else{
                *(polybius_table+i) = '\0';
            }
        }
    }

    char inputc = '\0';
    int input = 0;
    char encriped_r = '\0';
    char encriped_c = '\0';

    while((input=getc(stdin)) != EOF){
        inputc = (char) input;

        if ((inputc == '\t') || (inputc == ' ') || (inputc == '\n'))
        {
            putc(inputc, stdout);
        }else{
            int index = get_index_in_table(inputc);
            if (index == -1)
            {
                return EXIT_FAILURE;
            }

            int row_i = index/col;
            int col_i = index % col;

            encriped_r = int_to_char(row_i);
            encriped_c = int_to_char(col_i);

            putc(encriped_r, stdout);
            putc(encriped_c, stdout);
        }

    }

    return EXIT_SUCCESS;

}

unsigned int char_to_Index(char chr){
    unsigned int index = 0;
    if (chr == 'A')
    {
        index = 10;
    } else if (chr == 'B')
    {
        index = 11;
    } else if (chr == 'C')
    {
        index = 12;
    } else if (chr == 'D')
    {
        index = 13;
    } else if (chr == 'E')
    {
        index = 14;
    } else if (chr == 'F')
    {
        index = 15;
    } else {
        index = chr - '0';
    }

    return index;

}

unsigned int cipher_pd(unsigned short mode){

    // getting row
    unsigned int row_temp = mode & 0x00F0;//0x00F0 = 0000 0000 1111 0000. This keeps the 1111 part in mode.
    unsigned int row = row_temp/16; //shift back 4 bits. 2^4 = 16.

    // getting column

    unsigned int col = mode & 0x000F; //0x000f = 0000 0000 0000 1111. This keeps the leftmost 4 bits.

    unsigned int total_elements_in_table = row*col;
    //construct table.
    if (key != ((char*)NULL)){

        int i = 0;
        for (i = 0; *(key+i) != '\0'; i++)
        {
            *(polybius_table+i) = *(key+i);
        }

        for (int j = 0; *(polybius_alphabet+j)!='\0'; j++)
        {
            if (is_char_in_key(*(polybius_alphabet+j)) == FALSE)
            {
                *(polybius_table+i) = *(polybius_alphabet+j);
                i++;
            }
        }

        for (int k = i; k < total_elements_in_table; k++)
        {
            *(polybius_table+k) = '\0';
        }
    }else {

        int i=0;
        for (i = 0; i < total_elements_in_table; i++)
        {
            if (*(polybius_alphabet+i) != '\0')
            {
                *(polybius_table+i) = *(polybius_alphabet+i);
            }else{
                *(polybius_table+i) = '\0';
            }
        }
    }


    char inputc = '\0';
    int input = 0;
    char decrypted = '\0';
    char encriped_c = '\0';
    char encriped_r = '\0';

    while((input=getc(stdin)) != EOF){

        inputc = (char) input;

        if ((inputc == '\t') || (inputc == ' ') || (inputc == '\n'))
        {
            putc(inputc, stdout);

        }else{

            if (encriped_r == '\0')
            {
                encriped_r = inputc;
            }else if(encriped_c == '\0'){
                encriped_c = inputc;
            }

            if ((encriped_r != '\0') && (encriped_c != '\0'))
            {
                unsigned int row_i = char_to_Index(encriped_r);
                unsigned int col_i = char_to_Index(encriped_c);

                //reset
                encriped_r = '\0';
                encriped_c = '\0';

                unsigned int index = cal_Index(row_i, col_i, col);
                decrypted = *(polybius_table+index);
                putc(decrypted, stdout);
            }
        }
    }


    return EXIT_SUCCESS;


}

int get_morse_table_index(char chr){

    int index = chr - '!';
    return index;

}

//add morsed to buffer and return the index that after the last element.
int add_to_buffer(char* buffer, char* morsed, int buffer_index){

    //printf("morsed: %s\n", morsed);
    //printf("Before add: %s\n", buffer);

    //int buf_i = buffer_index;
    int i =0;
    for (i = 0; *(morsed +i) != '\0'; i++)
    {
        *(buffer+buffer_index+i) = *(morsed + i);
    }

    int index = buffer_index + i;

    //printf("After add: %s\n", buffer);



    return index;
}




int shift_buffer_back3(char* buffer){
    int buff_size = buffer_size(buffer);
    int limit = 257-3; //254
    int i =0;

    //printf("Before shift: %s\n", buffer);


    for (i = 0; i < limit; i++)
    {
        *(buffer+i) = *(buffer+i+3);
    }

    for (int j = i; j < 257; j++)
    {
        *(buffer+j) = '\0';
    }

    //printf("After shift: %s\n", buffer);

    int count=0;
    for (int c = 0; *(buffer+c) != '\0'; c++)
    {
        count++;
    }

    //count--;

    return count;
}


int shift_buffer_back1(char* buffer){
    int buff_size = buffer_size(buffer);
    int limit = 257-1; //254
    int i =0;

    //printf("Before shift: %s\n", buffer);


    for (i = 0; i < limit; i++)
    {
        *(buffer+i) = *(buffer+i+1);
    }

    for (int j = i; j < 257; j++)
    {
        *(buffer+j) = '\0';
    }

    //printf("After shift: %s\n", buffer);

    int count=0;
    for (int c = 0; *(buffer+c) != '\0'; c++)
    {
        count++;
    }

    //count--;

    return count;
}

int get_frac_table_index(char* fm_buff){
    int index = -1;
    for (int i = 0; i< 26; i++)
    {
        if (string_eql(((char*)(*(fractionated_table+i))), fm_buff) == TRUE)
        {
            index = i;
            return index;
        }
    }

    return index;
}




unsigned int cipher_fe(){
    //constrcut char *fm_key;
    //int length_of_fm_key = sizeof(fm_key)/sizeof(*(fm_key+0));

    if (key != (char*)NULL)
    {
        int i = 0;
        for (i = 0; *(key+i) != '\0'; i++)
        {
            *(fm_key+i) = *(key+i);
        }

        for (int j = 0; *(fm_alphabet+j)!='\0'; j++)
        {
            if (is_char_in_key(*(fm_alphabet+j)) == FALSE)
            {
                *(fm_key+i) = *(fm_alphabet+j);
                i++;
            }
        }

        for (int k = i; k < 27 ; k++) // IF the fm_key length isn't guaranteed to be 27, then, it should be changed.
        {
            *(fm_key+k) = '\0';
        }

    }else {

        int i=0;
        for (i = 0; i <27 ; i++)
        {
            if (*(fm_alphabet+i) != '\0')
            {
                *(fm_key+i) = *(fm_alphabet+i);
            }else{
                *(fm_key+i) = '\0';
            }
        }
    }

    //printf("fm_key: %s\n", fm_key);



    char* morse_code_buffer = polybius_table;
    //initialize all elements in polybius _table to '\0'.

    for (int i = 0; i < 257; i++)
    {
        *(morse_code_buffer+i) = '\0';
    }

    int morse_code_index = 0;
    char inputc = '\0';
    int input = 0;

    char char1 = '\0'; // previous
    long fm_space = 0; // 8 bytes for 64bits machine.s
    char *fm_buff = (char*) &fm_space;

    for (int i = 0; i < 4; i++)
    {
        *(fm_buff+i) = '\0';
    }

    char *morsed = (char*)NULL;
    int buffer_index = 0;

    while((input=getc(stdin)) != EOF){

        inputc = (char)input;


        if ((inputc != '\t') && (inputc != ' ') && (inputc != '\n'))
        {
            morse_code_index = get_morse_table_index(inputc);



            if (morse_code_index < 0)
            {
                return EXIT_FAILURE;
            }

            morsed = (char*)(*(morse_table + morse_code_index));


            //printf("%c: %s\n", inputc, morsed);



            if (*(morsed+0) == '\0')
            {
                //printf("FAILED: %s\n", morsed);

                return EXIT_FAILURE;
            }

            //printf("buffer_index: %i\n", buffer_index);
            buffer_index = add_to_buffer(morse_code_buffer, morsed, buffer_index);
            //printf("buffer_index: %i\n", buffer_index);
            char* x_flag = "x";
            buffer_index = add_to_buffer(morse_code_buffer, x_flag, buffer_index);
            //printf("%s\n", morse_code_buffer);
            //break;





            while (buffer_size(morse_code_buffer) >=3)
            {
                //take first 3 char from morse_code_buffer.
                *(fm_buff+0) = *(morse_code_buffer+0);
                *(fm_buff+1) = *(morse_code_buffer+1);
                *(fm_buff+2) = *(morse_code_buffer+2);

                //printf("%s\n", fm_buff);


                //TEST
                //printf("BEFORE SHIFT BACK: %s\n", morse_code_buffer);


                buffer_index = shift_buffer_back3(morse_code_buffer);

                //printf("AFTER SHIFT BACK: %s\n", morse_code_buffer);

                int fm_i = get_frac_table_index(fm_buff);
                if (fm_i == -1)
                {
                    //printf("%s%s\n", "fm_buff isn't found in fractionated_table: ",fm_buff);
                    //printf("morse_code_buffer: %s\n", morse_code_buffer);
                    return EXIT_FAILURE;
                }

                putc(*(fm_key+fm_i), stdout);

                for (int i = 0; i < 4; i++){
                    *(fm_buff+i) = '\0';
                }

            }

            char1 = inputc;

        }else{


            if ((is_char_in_alphabet(char1, fm_key)==TRUE) && ((inputc == ' ') || (inputc == '\t') || (inputc == '\n')) )
            {

                char* x_flag = "x";
                buffer_index = add_to_buffer(morse_code_buffer, x_flag, buffer_index);

                //buffer_index = add_to_buffer(morse_code_buffer, "x", buffer_index);
                while (buffer_size(morse_code_buffer) >=3)
                {
                    //printf("%s\n", morse_code_buffer);

                    //printf("%s%i\n", "buffer_size: ", buffer_size(morse_code_buffer));
                    //take first 3 char from morse_code_buffer.
                    *(fm_buff+0) = *(morse_code_buffer+0);
                    *(fm_buff+1) = *(morse_code_buffer+1);
                    *(fm_buff+2) = *(morse_code_buffer+2);

                    buffer_index = shift_buffer_back3(morse_code_buffer);
                    int fm_i = get_frac_table_index(fm_buff);
                    if (fm_i == -1)
                    {
                        //printf("%s%s\n", "fm_buff isn't found in fractionated_table: ",fm_buff);
                        //printf("morse_code_buffer: %s\n", morse_code_buffer);
                        return EXIT_FAILURE;
                    }

                    putc(*(fm_key+fm_i), stdout);

                    for (int i = 0; i < 4; i++){
                        *(fm_buff+i) = '\0';
                    }

                }

                char1 = inputc;

            }

            if (inputc == '\n')
            {
                putc(inputc, stdout);
                char1 = inputc;

                //clear x in the begining of the buffer.
                while (*(morse_code_buffer+0) == 'x')
                {
                    buffer_index = shift_buffer_back1(morse_code_buffer);
                }



                // //new
                // for (int i = 0; i < 257; i++)
                // {
                //      *(morse_code_buffer+i) = '\0';
                // }
            }
        }



    }//while



    return EXIT_SUCCESS;
}


int get_fm_key_index(char letter){
    int index = -1;

    for (int i = 0; i < 26; i++)
    {
        if (*(fm_key+i) == letter)
        {
            index = i;
            return index;
        }
    }

    return index;
}

unsigned int does_contain_x(char* buffer){
    unsigned int ans = FALSE;
    for (int i = 0; *(buffer+i) != '\0'; i++)
    {
        if (*(buffer+i) == 'x')
        {
            ans = TRUE;
            return ans;
        }
    }

    return ans;
}

int shift_buffer_backi(char* buffer, int amount){
    int buff_size = buffer_size(buffer);
    int limit = 257-amount; //254
    int i =0;

    //printf("Before shift: %s\n", buffer);
    if (*(buffer+0) == '\0')
    {
        return 0;
    }

    for (i = 0; i < limit; i++)
    {
        *(buffer+i) = *(buffer+i+amount);
    }

    for (int j = i; j < 257; j++)
    {
        *(buffer+j) = '\0';
    }

    //printf("After shift: %s\n", buffer);

    int count=0;
    for (int c = 0; *(buffer+c) != '\0'; c++)
    {
        count++;
    }

    //count--;

    return count;
}

int get_index_in_morse_table(char* morse_c_buff){
    //int i = -1;
    int limit = 'z' - '!'+1;
    char* element = (char*)NULL;

    for ( int i=0; i<limit; i++)
    {
        element = (char*)(*(morse_table+i));
        if (*(element+0) != '\0')
        {
            if (string_eql(element, morse_c_buff) == TRUE)
            {
                return i;
            }
        }
    }

    return -1;
}

int string_eql_p3(char* str1, char* str2){

    int chrc = 0;
    char char1 = *(str1+chrc);
    char char2 = *(str2+chrc);

    if (buffer_size(str1) != buffer_size(str2))
    {
        return 0;
    }

    while( char1 != '\0' && char2 != '\0'){
        if (char1 != char2)
        {
            return 0; //false
        }
        chrc ++;
        char1 = *(str1+chrc);
        char2 = *(str2+chrc);
    }

    return 1; //true
}

unsigned int cipher_fd(){

    //constrcut fm_key.
    if (key != (char*)NULL)
    {
        int i = 0;
        for (i = 0; *(key+i) != '\0'; i++)
        {
            *(fm_key+i) = *(key+i);
        }

        for (int j = 0; *(fm_alphabet+j)!='\0'; j++)
        {
            if (is_char_in_key(*(fm_alphabet+j)) == FALSE)
            {
                *(fm_key+i) = *(fm_alphabet+j);
                i++;
            }
        }

        for (int k = i; k < 27 ; k++) // IF the fm_key length isn't guaranteed to be 27, then, it should be changed.
        {
            *(fm_key+k) = '\0';
        }

    }else {

        int i=0;
        for (i = 0; i <27 ; i++)
        {
            if (*(fm_alphabet+i) != '\0')
            {
                *(fm_key+i) = *(fm_alphabet+i);
            }else{
                *(fm_key+i) = '\0';
            }
        }
    }

    //TEST
    //printf("fm_key:%s\n", fm_key);




    char* morse_code_buffer = polybius_table;
    //initialize all elements in polybius _table to '\0'.

    for (int i = 0; i < 257; i++)
    {
        *(morse_code_buffer+i) = '\0';
    }

    int morse_code_index = 0;
    char inputc = '\0';
    int input = 0;

    char char1 = '\0'; // previous
    long double fm_space = 0; // 8 bytes for 64bits machine.s
    char *fm_buff = (char*) &fm_space;

    //char *fm_buff = (char*) &fm_space;

    for (int i = 0; i < 4; i++)
    {
        *(fm_buff+i) = '\0';
    }

    char *morsed = (char*)NULL;
    int buffer_index = 0;

    long double space2 = 0; //12 bytes
    char* morse_c_buff = (char*) &space2;

    //initialize morse_c_buff

    for (int i = 0; i < 10; i++)
    {
        *(morse_c_buff+i) = '\0';
    }

    while((input=getc(stdin)) != EOF){
        inputc = (char) input;

        //get the index in fm_key.
        if (inputc == '\n')
        {
            putc(inputc, stdout);

        } else {



            int fm_i = get_fm_key_index(inputc);
            //printf("fm_i: %i\n", fm_i);

            if (fm_i == -1)
            {
            //NO VALID INPUT
                return EXIT_FAILURE;
            }
            morsed =(char*)(*(fractionated_table + fm_i));

            //TEST
            //printf("morsed: %s\n", morsed);



            buffer_index = add_to_buffer(morse_code_buffer, morsed, buffer_index);
            //TEST
            //printf("after add_to_buffer: %s\n", morse_code_buffer);




            while(does_contain_x(morse_code_buffer)== TRUE){
            // extract chars before 'x' and record the 'x' to char 1
            // convert the char sequences before 'x' to original char.
            // if char1 and current char are both 'x', output a ' '.


                //TEST
                //printf("morse_code_buffer:%s\n", morse_code_buffer);
                //printf("char1: %c\n", char1);

                if ((*(morse_code_buffer+0)=='x') && (*(morse_code_buffer+0)==char1))
                {

                    char space = ' ';

                    putc(space, stdout);
                    buffer_index = shift_buffer_back1(morse_code_buffer);
                    break;
                    //printf("after shift_buffer_back1: %s. buffer_index: %i\n", morse_code_buffer, buffer_index);
                }


                int i = 0;

                for (i = 0; *(morse_code_buffer+i) != 'x'; i++)
                {
                    *(morse_c_buff+i)= *(morse_code_buffer+i);
                }

                //printf("morse_c_buff: %s\n", morse_c_buff);

                char1 = *(morse_code_buffer+i);

                //TEST
                //printf("char1: %c\n", char1);
                //printf("morse_c_buff: %s\n", morse_c_buff);
                //printf("beofre morse_code_buffer shifts: %s\n", morse_code_buffer);

                int morse_c_buff_size = buffer_size(morse_c_buff);
                buffer_index = shift_buffer_backi(morse_code_buffer, morse_c_buff_size);
                buffer_index = shift_buffer_back1(morse_code_buffer);

                //TEST
                //printf("morse_c_buff_size: %i\n", morse_c_buff_size);
                //printf("after morse_code_buffer shifts: %s\n", morse_code_buffer);
                //printf("buffer_index now: %i\n", buffer_index);



                if (*(morse_c_buff+0) != '\0')
                {


                    //TEST
                    //printf("morse_c_buff: %s\n", morse_c_buff);


                    int ascii_i = get_index_in_morse_table(morse_c_buff);


                    //TEST
                    //printf("ascii_i: %i\n", ascii_i);


                    if (ascii_i == -1)
                    {
                        return EXIT_FAILURE;
                    }

                    ascii_i = '!'+ascii_i;
                    char ascii_c = (char) ascii_i;

                    //printf("ascii_c: %c\n", ascii_c);
                    putc(ascii_c, stdout);

                    for (int i = 0; i < 10; i++)
                    {
                        *(morse_c_buff+i) = '\0';
                    }
                }


            }



        }
    }




    return EXIT_SUCCESS;
}







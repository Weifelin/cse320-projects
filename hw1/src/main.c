#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


int main(int argc, char **argv)
{
    unsigned short mode;





    mode = validargs(argc, argv);

    //printf("MODE: 0x%X\n", mode);

    debug("Mode: 0x%X", mode);

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }
    if (mode == 0)
    {
        USAGE(*argv, EXIT_FAILURE);
    }

    //unsigned short cipher_mode = mode;

    //don't care last 13 bits since "mode" contains the info and will be used if needed.
    // pd/pe/fd/fe are just used to determine which function should be called.

    unsigned short pd = 0x2000; //0x2000 = 0010 0000 0000 0000. use AND.
    unsigned short pe = 0x0000;
    unsigned short fd = 0x6000;//0x6000 = 0110 0000 0000 0000.
    unsigned short fe = 0x4000;//0x4000 = 0100 0000 0000 0000.

    // AND with 1110 0000 0000 0000(0XE000) to preserve first 3 bits.

    unsigned int return_result = EXIT_SUCCESS;

    if ((mode & 0XE000) == pe)
    {
        return_result = cipher_pe(mode);
    }

    if ((mode & 0xE000) == pd)
    {
        return_result = cipher_pd(mode);
    }

    if ((mode & 0xE000) == fe)
    {
        return_result = cipher_fe();
    }

    if ((mode & 0xE000) == fd)
    {
        return_result = cipher_fd();
    }

    //return EXIT_SUCCESS;
    return return_result;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
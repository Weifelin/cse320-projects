#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();


    //double* ptr = sf_malloc(sizeof(double));

    //*ptr = 320320320e-320;

    //printf("%f\n", *ptr);
    //printf("%li\n",(long) ptr);

    //sf_free(ptr);

    void *x = sf_malloc(sizeof(double));

    size_t page = 4096;

    void* y= sf_malloc(page);

    //sf_snapshot();


    //void* z = sf_malloc(sizeof(int));
    sf_free(y);
    //printf("%s\n", "Here");
    sf_free(x);





    sf_mem_fini();

    return EXIT_SUCCESS;
}

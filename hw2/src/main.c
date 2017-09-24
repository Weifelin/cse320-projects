#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <stdlib.h>
//#include "args.c"
//#include "utf.c"

int main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */
  get_encoding_function()(infile, outfile);
  //get_encoding_function(infile, outfile);//added

  if(program_state != NULL) {
    //close((int)program_state);
    close(infile);//added
    close(outfile);//added
  }

  free(program_state);
  //I think this is how this works //original
  // free((void*)outfile);
  // free((void*)infile);
  return EXIT_SUCCESS;
}

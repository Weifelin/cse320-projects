#include "utf.h"
#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf16be_to_utf16le(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  ssize_t bytes_to_write;
  int ret = -1;

  bom = UTF16LE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 3);
#endif
  write_to_bigendian(outfile, &bom, 2);

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    bytes_to_write = 4; /* utf-32 future compatibility */
    reverse_bytes(&buf.upper_bytes, 2);
    if(is_lower_surrogate_pair(buf)) {
      if((bytes_read = read_to_bigendian(infile, &buf.lower_bytes, 2) > 0)) {
        break;
      }
      reverse_bytes(&(buf.lower_bytes), 2);
      bytes_to_write += 2;
    }
    write_to_bigendian(outfile, &bom, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}



int
from_utf16be_to_utf8(int infile, int outfile)
{

  int inode_in, inode_out;
  struct stat file_stat;

  int ret_in = fstat(infile, &file_stat);
  if (ret_in<0)
  {
    exit(EXIT_FAILURE);
  }
  inode_in = file_stat.st_ino;

  int ret_out = fstat(outfile, &file_stat);
  if (ret_out<0)
  {
    exit(EXIT_FAILURE);
  }

  inode_out = file_stat.st_ino;

  if (inode_in == inode_out)
  {
    exit(EXIT_FAILURE);
  }

  int ret = -1;
 /* int bom;
  utf8_glyph_t utf8_buf;
  ssize_t bytes_read;
  size_t remaining_bytes;
  size_t size_of_glyph;
  ssize_t bytes_to_write;

  code_point_t code_point;
  utf16_glyph_t utf16_buf;

  bom = UTF8;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 4);
#endif
  write_to_bigendian(outfile, &bom, 3);


  while ((bytes_read = read_to_bigendian(infile, &(utf16_buf.upper_bytes), 2)) > 0) {
    code_point
  }*/



  return ret;
}

utf16_glyph_t
code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;

  memeset(&ret, 0, sizeof ret);
  if(is_code_point_surrogate(code_point)) {
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
    reverse_bytes(&ret.lower_bytes, 2);
  #endif
    *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
  #endif
    *size_of_glyph = 2;
  }
  return ret;
}

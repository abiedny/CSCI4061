// nftw_listing.c: Demonstrates use of the nftw() library call to
// recursively travere directory. Each file visited has a function
// called on it.

#include "headers.h"

#define MAX_FDS  20             // maximum file descriptors to be used by nftw()

int main(int argc, char *argv[]) {
  char *filename = ".";                           // default to traversing current directory
  if(argc > 1){                                   // or take command line argument as dir to list
    filename = argv[1];
  }
  printf("%c %-24s %-8s %s\n",                    // print a header for information columns
         'T', "MTIME","BYTES","FILENAME");
  printf("%c %-24s %-8s %s\n",                    
         '=', "========================", "========", "================");
  int len = strlen(filename);
  if(filename[len-1] == '/'){                     // chop trailing slash if present
    filename[len-1] = '\0';
  }

  nftw(filename, print_file_info,                 // library call to recursively traverse and
       MAX_FDS, FTW_PHYS);                        // call print_file_info() on each entry

  printf("========================================\n");
  printf("%d total files visited\n",total_files);
  return 0;
}

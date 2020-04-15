// print_file_info.c: helper function which prints information on a
// single file.

#include "headers.h"

int total_files = 0;            // tracks total files visited

// Prints info on a single file indicated by argument
// 'filename'. Argument 'sb' is a pointer to a 'struct stat' that has
// already been filled with information for the file via a stat()-like
// call. The 'ignore1' and 'ignore2' arguments are for compatibilty
// with the library function 'nftw()'.
int print_file_info(const char *filename, const struct stat *sb,
                    const int ignore1,    struct FTW *ignore2)
{
  // check file type type using S_xxx macros. These are associated
  // with stat() family of calls and use the bits in field sb.st_mode
  // to report the kind of file found.
  char type = '?';                                // char to set for type
       if( S_ISDIR (sb->st_mode) ){ type = 'D'; } // Directory
  else if( S_ISREG (sb->st_mode) ){ type = 'F'; } // Regular File
  else if( S_ISLNK (sb->st_mode) ){ type = 'L'; } // Symbolic Link
  else if( S_ISBLK (sb->st_mode) ){ type = 'B'; } // Block special device (hard drive)
  else if( S_ISCHR (sb->st_mode) ){ type = 'C'; } // Character special device (terminal)
  else if( S_ISFIFO(sb->st_mode) ){ type = 'P'; } // Pipe / FIFO (via mkfifo)
  else if( S_ISSOCK(sb->st_mode) ){ type = 'S'; } // Socket (network device)
         
  // // alternate version which uses switch() control structure and
  // // masking to extract format bits
  // switch (sb->st_mode & S_IFMT) {
  //   case S_IFDIR:  type = 'D';      break;
  //   case S_IFREG:  type = 'F';      break;
  //   case S_IFLNK:  type = 'L';      break;
  //   case S_IFBLK:  type = 'B';      break;
  //   case S_IFCHR:  type = 'C';      break;
  //   case S_IFIFO:  type = 'P';      break;
  //   case S_IFSOCK: type = 'S';      break;
  //   default: printf("Unknown file type?\n"); break;
  // }

  char *time_str = ctime(&(sb->st_mtime));        // format modification time as a string
  time_str[strlen(time_str)-1] = '\0';            // eliminate trailing newline in formatted time
  printf("%c %24s %8lu %s\n",                     // print type of file and filename to it
         type, time_str, sb->st_size, filename);  

  total_files++;
  return 0;
}

// Add on for each file that is visited.
int sum_files(const char *filename, const struct stat *sb,
              const int ignore1,    struct FTW *ignore2)
{
  total_files++;
  return 0;
}

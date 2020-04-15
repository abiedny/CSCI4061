// recursive_listing.c: Demonstrates a 'by-hand' recursive traversal
// of a directory using opendir() and readdir() system calls. Also
// demonstrates the lstat() system call, similar to stat() but reports
// symbolic links as links rather than chasing them to the target file.

#include "headers.h"

#define NAMELEN 2048                              // maximum supported path length to files

// Recursively print info on all files starting at 'filename'.  If
// 'filename' is a normal file, only its information is printed. If it
// is a directory, the directory info is printed and the the directory
// is recursively traversed.
void recursive_print(char *filename){
  struct stat sb;                                 // stores data on a file
  // stat(filename, &sb);                         // chases symoblic links; dangerous as there may be cycles
  lstat(filename, &sb);                           // reports sybmolic links rather than chasing them

  print_file_info(filename, &sb, 0, NULL);        // print info on current file

  if( !S_ISDIR(sb.st_mode) ){                     // check whether current file is a directory
    return;                                       // not a directory, base case of recursion.
  }

  // Current file is a directory. Recurse on its contents.
  DIR *dir = opendir(filename);                   // open to scan through directory

  struct dirent *file = NULL;
  while( (file = readdir(dir)) != NULL){          // get a file from the directory
    if(strcmp(file->d_name,".") ==0 ||            // check for and ignore . and ..
       strcmp(file->d_name,"..")==0)              // (current / parent directories)
    {
      continue;                                   // next loop iteration: skips . and ..
    }
    char nextfilename[NAMELEN];                   // construct a next filename to visit
    snprintf(nextfilename,NAMELEN,"%s/%s",        // using snprintf()
             filename, file->d_name); 
    recursive_print(nextfilename);                // recurse on next file
  }
  closedir(dir);
  return;
}

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
  recursive_print(filename);                      // begin recursion
  printf("========================================\n");
  printf("%d total files visited\n",total_files);
  return 0;
}

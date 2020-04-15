#define _XOPEN_SOURCE 700       // causes some functions like nftw() to become defined in headers
#include <ftw.h>                // includes nftw() function and associated structs
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

// print_file_info.c
extern int total_files;
int print_file_info(const char *filename, const struct stat *sb,
                    const int ignore1,    struct FTW *ignore2);
int sum_files(const char *filename, const struct stat *sb,
              const int ignore1,    struct FTW *ignore2);

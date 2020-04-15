// qsort_demo.c: demonstrates use of the C standard qsort() library
// function to sort arbitrary data.  qsort() take a Comparison
// Function along with pointers and sizes of the data to sort.  This
// enables it to sort arbitrary data with the same internal
// algorithm. Writing and passing correctly typed comparsion functions
// is a bit tricky and this code demonstrates one technique to do so
// including how to caste library functions like strcmp() to be used
// in qsort() to sort stings.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compare two ints pointed to
int intcmp(const int *xp, const int *yp) { 
  return *xp - *yp;
}

// compare two ints pointed to, reverse order
int rev_intcmp(const int *xp, const int *yp) { 
  return *yp - *xp;
}

// strcmp() can be used for sorting strings in standard order

// compare strings in reverse order
int rev_strcmp(const char *xp, const char *yp) { 
  return -strcmp(xp,yp);
}

// convenience type to enable casting of typed comparisong functions
// to the void-ish functions expected by qsort()
typedef int (*cmp_t) (const void *, const void *);

int main() { 
    int int_arr[10] = {1, 6, 5, 2, 3, 9, 4, 7, 8, 0}; 

    qsort((void*) int_arr, 10, sizeof(int), (cmp_t) intcmp); 
    for (int i = 0; i < 10; ++i) {
      printf("%d ", int_arr[i]);
    }
    printf("\n");
    
    qsort((void*) int_arr, 10, sizeof(int), (cmp_t) rev_intcmp); 
    for (int i = 0; i < 10; ++i) {
      printf("%d ", int_arr[i]);
    }
    printf("\n");
  
    char *str_arr[5] = {"banana", "orange", "peach", "apple", "grape"};
    qsort((void*) str_arr, 5, sizeof(char *), (cmp_t) strcmp); 
    for (int i = 0; i < 5; ++i) {
      printf("%s ", str_arr[i]);
    }
    printf("\n");

    qsort((void*) str_arr, 5, sizeof(char *), (cmp_t) rev_strcmp); 
    for (int i = 0; i < 5; ++i) {
      printf("%s ", str_arr[i]);
    }
    printf("\n");

    return 0; 
} 

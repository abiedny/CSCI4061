// worm_pthreads.c: demonstrates multiple thread running at different
// rates, all modifying the same global variables that represent a
// "board". The board is repeatedly printed in the same area to show
// the progress of the worms.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#define MAX_ROW    10                            // size of board in rows
#define MAX_COL    10                            // size of board in cols
#define NSTEPS     50                            // number of steps worms take
#define NWORMS      4                            // run 4 worm threads
  
char board[MAX_ROW+1][MAX_COL+1];                // board: 2D array of characters, +1 for newline/null chars
pthread_mutex_t mutexes[MAX_ROW][MAX_COL];       // board: array of locks for each cell 

typedef struct {                                 // worm data
  char showchar;                                 // character to show for worm
  double delay;                                  // delay between moves in seconds 
  int row, col;                                  // current row/col
  int steps;                                     // how many steps the worm has taken
  int territory;                                 // number of cells of territory that the work occupies
} worm_t;

worm_t worms[NWORMS] = {                         // 4 worms, different params for each
  {.showchar='E', .delay=0.50, .row=0,         .col=0,         .steps=0, .territory=0,},
  {.showchar='A', .delay=0.25, .row=MAX_ROW-1, .col=0,         .steps=0, .territory=0,},
  {.showchar='G', .delay=0.60, .row=MAX_ROW-1, .col=MAX_COL-1, .steps=0, .territory=0,},
  {.showchar='L', .delay=0.10, .row=0,         .col=MAX_COL-1, .steps=0, .territory=0,},
};

void *worm_func(void *worm);                     // function worm threads run, takes a worm_t pointer
void *print_func(void *null);                    // function print thread rusn, no args taken
void handler(int signum);                        // handler for interrupts to restore terminal settings

int main(){
  signal(SIGINT,  handler);                      // handle signals to restore terminal settings
  signal(SIGTERM, handler);

  for(int i=0; i<MAX_ROW; i++){                  // initialized board/mutex locks 
    for(int j=0; j<MAX_COL; j++){
      board[i][j] = '.';                         // dots fill board initially
      pthread_mutex_init(&mutexes[i][j],NULL);   // initialize lock
    }
    board[i][MAX_COL] = '\n';                    // end each board row with a newline
  }
  board[MAX_ROW][0] = '\0';                      // board treated as single string, null terminate

  printf("\33[2J\33[1;1H");                      // erase screen, reset cursor pos to upper left, see note at bottom
  printf("Running worm threads...\n");

  pthread_t print_thread;                        // data for printing thread
  pthread_create(&print_thread, NULL,            // start the printing thread running
                 print_func, NULL);
  pthread_t worm_threads[NWORMS];                // array of thread types for the worm threads
  for(int i=0; i < NWORMS; i++){                 // parent generates child threads
    pthread_create(&worm_threads[i], NULL,
                   worm_func, &worms[i]);
  }

  for(int i=0; i < NWORMS; i++){                 // main thread waits on all children to finish
    pthread_join(worm_threads[i], NULL);
  }
  pthread_join(print_thread, NULL);

  printf("All threads complete.\n");

  for(int i=0; i<MAX_ROW; i++){                  // deallocate the mutexes
    for(int j=0; j<MAX_COL; j++){
      pthread_mutex_destroy(&mutexes[i][j]);
    }
  }

  return 0;
}

// Function run by each thread to produce 'worm' output
void *worm_func(void *wp){                                      // function which moves a 'worm' around in the file
  worm_t *worm = (worm_t *) wp;                                 // caste parameter from void

  board[worm->row][worm->col] = worm->showchar;                 // place the initial worm character on the board
  worm->territory = 1;                                          // worms start with 1 position

  int moves[4][2] = {                                           // 4 possible moves, will iterate over these and
    {+1, 0}, {-1, 0}, { 0,-1}, { 0,+1},                         // adopt the first one that works
                                                                // up     down      left     right
  };

  unsigned int state = time(NULL);                              // initialize private state for random number generation

  for(worm->steps=0; worm->steps < NSTEPS; worm->steps++){      // loop over # steps
    int movei = rand_r(&state) % 4;                             // select a random move (u/d/l/r) to start with
    for(int i=0; i<4; i++, movei=(movei+1)%4){                  // loop over all possible moves adopting the first one that works
      int new_row = worm->row + moves[movei][0];                // calculate possible next position
      int new_col = worm->col + moves[movei][1];

      if(   new_row < 0 || new_row >= MAX_ROW                   // check if current move (u/d/l/r) is out of bounds
         || new_col < 0 || new_col >= MAX_COL)
      {
        continue;                                               // out of bounds: loop back around and try different move
      }

      pthread_mutex_lock(&mutexes[new_row][new_col]);           // lock the inbounds position to prevent other worms from changing it
      if(   board[new_row][new_col] != '.'                      // check if it is available or...
         && board[new_row][new_col] != tolower(worm->showchar)) // part of this worm's territory
      {
        pthread_mutex_unlock(&mutexes[new_row][new_col]);       // nope, unlock position and 
        continue;                                               // loop back around and try another move
      }

      if(board[new_row][new_col] == '.'){                       // got an empty spot, increase territory
        worm->territory++;
      }
       
      board[worm->row][worm->col] = tolower(worm->showchar);    // found a valid location, lowercase old position
      worm->row = new_row; worm->col = new_col;                 // update current position
      board[worm->row][worm->col] = worm->showchar;             // mark new position
      pthread_mutex_unlock(&mutexes[new_row][new_col]);         // unlock position
      break;                                                    // made a valid move, bail out
    }
    usleep((int) (1e6 * worm->delay));                          // sleep until trying another move
  }

  return NULL;                                                  // thread returns null
}

// Function run by the thread that prints the board and worm info
void *print_func(void *null){                    // takes no real data
  printf("\33[s");                               // save cursor position
  printf("\33[?25l");                            // hide cursor to avoid flicker
  int ndone = 0;
  while(ndone < NWORMS){                         // while worms are running...
    printf("\33[u");                             // restore cursor position
    usleep(2000);                                // sleep a moment
    printf("%s\n", (char*)board);                // print the board as a single string
    ndone = 0;
    for(int i=0; i<NWORMS; i++){                 // print all worm information
      worm_t w = worms[i];
      printf("%c @ (%2d,%2d) : step %3d / %3d territory: %2d\n",
             w.showchar,w.row,w.col,w.steps,NSTEPS,w.territory);
      ndone += (w.steps >= NSTEPS);              // count worms that are finished
    }
    printf("%d worms finished\n",ndone);
  }
  printf("\33[?25h");                            // show cursor
  return NULL;
}

// Signal handler that restores cursor/position on Ctl-c
void handler(int sigum){                         // handle signals on sigint/sigterm
  printf("\33[?25h");                            // show cursor again
  for(int i=0; i<NWORMS+MAX_ROW+3; i++){         // move the cursor down below display
    printf("\n");
  }
  exit(1);                                       // bail out
}

// NOTE: the program uses some wacky "terminal control escape
// sequences" to repeatdly print the worm board in the same
// position. Example:
// 
// printf("\33[2J\33[1;1H");   // erase screen, reset cursor pos to upper left
// printf("\33[?25l");         // hide cursor to avoid flicker
// printf("\33[?25h");         // show cursor again
// 
// Read more at https://en.wikipedia.org/wiki/ANSI_escape_code

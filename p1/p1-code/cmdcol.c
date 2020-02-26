// cmdcol.c: functions related to cmdcol_t collections of commands.

#include "commando.h"

// Add the given cmd to the col structure. Update the cmd[] array and
// size field. Report an error if adding would cause size to exceed
// MAX_CMDS, the maximum number commands supported.
void cmdcol_add(cmdcol_t *col, cmd_t *cmd) {
    if (col->size >= MAX_CMDS) {
        eprintf("Too many commands in command collection");
        return;
    }
    col->cmd[col->size] = cmd;
    col->size++;
}

// Print all cmd elements in the given col structure.  The format of
// the table is
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 0    #17434       0    EXIT(0) 2239 ls -l -a -F 
// 1    #17435       0    EXIT(0) 3936 gcc --help 
// 2    #17436      -1        RUN   -1 sleep 2 
// 3    #17437       0    EXIT(0)  921 cat Makefile 
// 
// Widths of the fields and justification are as follows
// 
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 1234 #12345678 1234 1234567890 1234 Remaining
// left  left    right      right rigt left
// int   int       int     string  int string
//
// The final field should be the contents of cmd->argv[] with a space
// between each element of the array.
void cmdcol_print(cmdcol_t *col) {
    char full_command[MAX_LINE];
    strcat(full_command, "");
    printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n");
    for (int i = 0; i < col->size; i++) {
        memset(full_command, '\0', MAX_LINE);
        int j = 0;
        while(1) {
            if (col->cmd[i]->argv[j] == NULL) break;
            strcat(full_command, col->cmd[i]->argv[j]);
            strcat(full_command, " ");
            j++;
        }

        printf("%-4d #%-8d %4d %10s %4d %-s\n", i, col->cmd[i]->pid, col->cmd[i]->status,
                col->cmd[i]->str_status, col->cmd[i]->output_size, full_command);
    }
}

// Update each cmd in col by calling cmd_update_state() which is also
// passed the block argument (either NOBLOCK or DOBLOCK) 
void cmdcol_update_state(cmdcol_t *col, int block) {
    for (int i = 0; i < col->size; i++) {
        cmd_update_state(col->cmd[i], block);
    }
}

// Call cmd_free() on all of the constituent cmd_t's.
void cmdcol_freeall(cmdcol_t *col) {
    for (int i = 0; i < col->size; i++) cmd_free(col->cmd[i]);
}
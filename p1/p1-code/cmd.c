// cmd.c: functions related the cmd_t struct abstracting a
// command. Most functions maninpulate cmd_t structs.

#include "commando.h"

// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.
cmd_t *cmd_new(char *argv[]) {
    char *_argv[ARG_MAX + 1];
    memset(_argv, '\0', sizeof(_argv));
    int i;
    for (i = 0; i < (ARG_MAX + 1); i++) {
        if (argv[i] == NULL) {
            if (_argv[i] != NULL) _argv[i] = '\0';
            break;
        }
        _argv[i] = strdup(argv[i]);
    }

    cmd_t *retVal = (cmd_t*)malloc(sizeof(cmd_t) );
    //memcpy(retVal->name, _argv[0], sizeof(_argv[0]) );
    strcpy(retVal->name, _argv[0]);
    memcpy(retVal->argv, _argv, sizeof(_argv) );

    retVal->finished = 0;
    sprintf(retVal->str_status, "INIT");
    retVal->pid = -1;
    retVal->out_pipe[0] = -1;
    retVal->out_pipe[1] = -1;
    retVal->status = -1;
    retVal->output = NULL;
    retVal->output_size = -1;

    if (retVal->argv[i] != NULL) {
        retVal->argv[i+1] = '\0';
    }

    return retVal;
}

// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.
void cmd_free(cmd_t *cmd) {
    for (int i = 0; i < NAME_MAX; i++) {
        if (cmd->argv[i] == NULL) break;
        free(cmd->argv[i]);
    }
    if (cmd->output != NULL) free(cmd->output);
    free(cmd);
}

// Forks a process and starts executes command in cmd in the process.
// Changes the str_status field to "RUN" using snprintf().  Creates a
// pipe for out_pipe to capture standard output.  In the parent
// process, ensures that the pid field is set to the child PID. In the
// child process, directs standard output to the pipe using the dup2()
// command. For both parent and child, ensures that unused file
// descriptors for the pipe are closed (write in the parent, read in
// the child).
void cmd_start(cmd_t *cmd) {
    sprintf(cmd->str_status, "RUN");
    pipe(cmd->out_pipe);

    pid_t child = fork();
    if (child == 0) {
        dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);
        close(cmd->out_pipe[PREAD]);
        execvp(cmd->name, cmd->argv);
    }
    cmd->pid = child;
    close(cmd->out_pipe[PWRITE]);
}

// If the finished flag is 1, does nothing. Otherwise, updates the
// state of cmd.  Uses waitpid() and the pid field of command to wait
// selectively for the given process. Passes block (one of DOBLOCK or
// NOBLOCK) to waitpid() to cause either non-blocking or blocking
// waits.  Uses the macro WIFEXITED to check the returned status for
// whether the command has exited. If so, sets the finished field to 1
// and sets the cmd->status field to the exit status of the cmd using
// the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
// output buffer for later printing.
//
// When a command finishes (the first time), prints a status update
// message of the form
//
// @!!! ls[#17331]: EXIT(0)
//
// which includes the command name, PID, and exit status.
void cmd_update_state(cmd_t *cmd, int block) {
    if (cmd->finished == 1) return;
    int status;
    waitpid(cmd->pid, &status, block);
    if (WIFEXITED(status)) { //TODO: Soemhow fix that fact that WIFEXITED always returns 1, even on sleep 99999....
        cmd->finished = 1;
        cmd->status = WEXITSTATUS(status);
        cmd_fetch_output(cmd);
        printf("@!!! %s[#%d]: EXIT(%d)\n", cmd->name, cmd->pid, cmd->status);
        sprintf(cmd->str_status, "EXIT(%d)", cmd->status);
    }
}

// Reads all input from the open file descriptor fd. Stores the
// results in a dynamically allocated buffer which may need to grow as
// more data is read.  Uses an efficient growth scheme such as
// doubling the size of the buffer when additional space is
// needed. Uses realloc() for resizing.  When no data is left in fd,
// sets the integer pointed to by nread to the number of bytes read
// and return a pointer to the allocated buffer. Ensures the return
// string is null-terminated. Does not call close() on the fd as this
// is done elsewhere.
char *read_all(int fd, int *nread) {
    size_t size = 1024*sizeof(char);
    char *buffer = (char*)malloc(size);
    memset(buffer, '\0', size);
    int _nread = 0;
    *nread = 0;
    int cur_pos = 0;
    while (1) {
        int max_read = size - cur_pos;
        _nread = read(fd, buffer + cur_pos, max_read); //
        cur_pos += _nread;

        if (_nread == 0) break;
        else if (_nread == -1) { 
            eprintf("Unable to read from fd %d\n", fd);
            exit(-1);
        }
        size = size * 2;
        buffer = realloc(buffer, size); //
        memset(buffer+(size/2), '\0', (size/2));
    }
    if (buffer[cur_pos] != '\0') {
        /*if (cur_pos == size) {
            buffer = realloc(buffer, sizeof(buffer) + 1);
        }*/
        buffer[cur_pos] = '\0';
    }
    (*nread) = cur_pos;
    return buffer;
}

// If cmd->finished is zero, prints an error message with the format
// 
// ls[#12341] not finished yet
// 
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.
void cmd_fetch_output(cmd_t *cmd) {
    if (!cmd->finished) {
        printf("%s[#%d] not finished yet", cmd->name, cmd->pid);
        return;
    }
    int nread = 0;
    cmd->output = read_all(cmd->out_pipe[PREAD], &nread);
    cmd->output_size = nread;
    close(cmd->out_pipe[PREAD]);
}

// Prints the output of the cmd contained in the output field if it is
// non-null. Prints the error message
// 
// ls[#17251] : output not ready
//
// if output is NULL. The message includes the command name and PID.
void cmd_print_output(cmd_t *cmd) {
    if (cmd->output == NULL) printf("%s[#%d] : output not ready", cmd->name, cmd->pid);
    else {
        write(STDOUT_FILENO, cmd->output, cmd->output_size);
    }
}
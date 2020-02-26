#include "commando.h"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
    int echo = 0;
    if (argc > 1) if (!strncmp(argv[1], "--echo", 6) || getenv("COMMANDO_ECHO") != NULL) echo = 1;

    cmdcol_t *cmdcol = malloc(sizeof(cmdcol_t));
    cmdcol->size = 0;

    char incmd[MAX_LINE];
    char *tokens[NAME_MAX];
    int ntokens = 0;
    while (1) {
        cmdcol_update_state(cmdcol, NOBLOCK);
        printf("@> ");
        if(fgets(incmd, MAX_LINE, stdin) == NULL) {
            printf("\nEnd of input");
            break;
        }
        if (echo) printf("%s", incmd);
        parse_into_tokens(incmd, tokens, &ntokens);
        if (!ntokens) continue;
        
        //search for builtins
        if (!strncmp(tokens[0], "help", 4)) {
            printf("COMMANDO COMMANDS\n\
help               : show this message\n\
exit               : exit the program\n\
list               : list all jobs that have been started giving information on each\n\
pause nanos secs   : pause for the given number of nanseconds and seconds\n\
output-for int     : print the output for given job number\n\
output-all         : print output for all jobs\n\
wait-for int       : wait until the given job number finishes\n\
wait-all           : wait for all jobs to finish\n\
command arg1 ...   : non-built-in is run as a job\n");
        }
        else if (!strncmp(tokens[0], "exit", 4)) break;
        else if (!strncmp(tokens[0], "list", 4)) cmdcol_print(cmdcol);
        else if (!strncmp(tokens[0], "pause", 5)) pause_for(atoi(tokens[1]), atoi(tokens[2]));
        else if (!strncmp(tokens[0], "output-for", 10)) {
            printf("@<<< Output for %s[#%d] (%d bytes):\n----------------------------------------\n", cmdcol->cmd[atoi(tokens[1])]->name, cmdcol->cmd[atoi(tokens[1])]->pid, cmdcol->cmd[atoi(tokens[1])]->output_size);
            cmd_print_output(cmdcol->cmd[atoi(tokens[1])]);
            printf("----------------------------------------\n");
        }
        else if (!strncmp(tokens[0], "output-all", 10)) {
            for (int i = 0; i < cmdcol->size; i++) {
                printf("@<<< Output for %s[#%d] (%d bytes):\n----------------------------------------\n", cmdcol->cmd[i]->name, cmdcol->cmd[i]->pid, cmdcol->cmd[i]->output_size);
                cmd_print_output(cmdcol->cmd[i]);
                printf("----------------------------------------\n");
            }
        }
        else if (!strncmp(tokens[0], "wait-for", 8)) {
            cmd_update_state(cmdcol->cmd[atoi(tokens[1])], DOBLOCK);
        }
        else if (!strncmp(tokens[0], "wait-all", 8)) {
            cmdcol_update_state(cmdcol, DOBLOCK);
        }
        else {
            cmd_t *new = cmd_new(tokens);
            cmdcol_add(cmdcol, new);
            cmd_start(new);
        }
    }

    cmdcol_freeall(cmdcol);
    free(cmdcol);
    return 0;
}
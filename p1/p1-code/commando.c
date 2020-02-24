#include "commando.h"

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
    int echo = 0;
    if (argv[1] == "--echo" || getenv("COMMANDO_ECHO") != NULL) echo = 1;

    char incmd[MAX_LINE];
    char *tokens[NAME_MAX];
    int ntokens = 0;
    while (1) {
        printf("@>");
        fgets(incmd, MAX_LINE, stdin);
        if (echo) printf(incmd);
        parse_into_tokens(incmd, tokens, &ntokens);
    }
 
    return 0;
}
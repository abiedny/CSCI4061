#include "blather.h"

int signalled = 0;
void sig_handler(int sig_num) {
    signalled = 1;
}

int main() {
    struct sigaction my_sa = {}; // portable signal handling setup with sigaction()
    sigemptyset(&my_sa.sa_mask); // don't block any other signals during handling
    my_sa.sa_flags = SA_RESTART; // always restart system calls on signals possible 
    my_sa.sa_handler = sig_handler; // run function handle_SIGTERM
    sigaction(SIGTERM, &my_sa, NULL); // register SIGTERM with given action
    my_sa.sa_handler = sig_handler; // run function handle_SIGINT
    sigaction(SIGINT,  &my_sa, NULL); // register SIGINT with given action

    server_t server;
    server_start(&server, "kevin", DEFAULT_PERMS);
    while(!signalled) {
        printf("Main loop...s");
        fflush(stdout);
        server_check_sources(&server); //check all sources
        if (server_join_ready(&server))  server_handle_join(&server); // handle join if ready
        for (int i = 0; i < server.n_clients; i++) { //foreach client, handle if ready
            if (server_client_ready(&server, i)) server_handle_client(&server, i);
        }
        printf("Main loop...e");
        fflush(stdout);
    }
    server_shutdown(&server);
    return 0;
}
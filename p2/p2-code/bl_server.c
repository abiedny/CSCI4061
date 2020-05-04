#include "blather.h"

server_t server;

int signalled = 0;
void sig_handler(int sig_num) {
    printf("Signalled, shutting down...");
    signalled = 1;
}

int ticked = 0;
void alarm_handler(int sig_num) {
    ticked = 1;
    alarm(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Give a name for the server...\n");
        exit(0);
    }

    //SIGALRM every second for ticks
    alarm(1);

    //setup signal handlers
    struct sigaction my_sa = {}; // portable signal handling setup with sigaction()
    sigemptyset(&my_sa.sa_mask); // don't block any other signals during handling
    my_sa.sa_flags = SA_RESTART; // always restart system calls on signals possible 
    my_sa.sa_handler = sig_handler; // run function handle_SIGTERM
    sigaction(SIGTERM, &my_sa, NULL); // register SIGTERM with given action
    my_sa.sa_handler = sig_handler; // run function handle_SIGINT
    sigaction(SIGINT,  &my_sa, NULL); // register SIGINT with given action
    my_sa.sa_handler = alarm_handler; // run function handle_SIGINT
    sigaction(SIGINT,  &my_sa, NULL); // register SIGINT with given action

    memset(&server, '\0', sizeof(server_t));
    server_start(&server, argv[1], DEFAULT_PERMS);

    printf("Server \"%s\" started up...\n", argv[1]);
    while(!signalled) {
        printf("Main loop...s\n");
        fflush(stdout);
        if (ticked) {
            ticked = 0;
            server_tick(&server);
            server_remove_disconnected(&server, 10);
        }
        server_check_sources(&server); //check all sources
        if (server_join_ready(&server))  server_handle_join(&server); // handle join if ready
        for (int i = 0; i < server.n_clients; i++) { //foreach client, handle if ready
            if (server_client_ready(&server, i)) server_handle_client(&server, i);
        }
        printf("Main loop...e\n");
        fflush(stdout);
    }
    server_shutdown(&server);
    return 0;
}
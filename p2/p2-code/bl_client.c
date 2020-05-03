#include "blather.h"

char to_fifo_name[MAXNAME];
int to_fifo_fd;
char from_fifo_name[MAXNAME];
int from_fifo_fd;

simpio_t simpio;
char name[MAXNAME];

pthread_t user_thread;
pthread_t server_thread;

void *user_thread_func(void *null) {
    while(!simpio.end_of_input) {
        //read input with simpio
        simpio_reset(&simpio);
        iprintf(&simpio, "");
        while(!simpio.line_ready && !simpio.end_of_input){
            simpio_get_char(&simpio);
        }
        if (simpio.line_ready) {
            //if line ready, create mesg_t and write to to_server fifo
            mesg_t mesg;
            mesg.kind = BL_MESG;
            strcpy(mesg.name, name);
            strcpy(mesg.body, simpio.buf);
            write(to_fifo_fd, &mesg, sizeof(mesg_t));
        }
        if (simpio.end_of_input) {
            //if end of input, write departed to server and kill user thread
            mesg_t bye;
            bye.kind = BL_DEPARTED;
            strcpy(bye.name, name);
            write(to_fifo_fd, &bye, sizeof(mesg_t));
        }
    }
    pthread_cancel(server_thread);
    return NULL;
}

void *server_thread_func(void *null) {
    while(1) {
        //read mesg_t from to_client fifo
        mesg_t rec;
        read(from_fifo_fd, &rec, sizeof(mesg_t));
        //if shutdown received, cancel user thread and then this
        if (rec.kind == BL_SHUTDOWN) break;
        //print with simpio
        iprintf(&simpio, "[%s] : %s\n", rec.name, rec.body);
    }
    //cancel user thread before you go
    pthread_cancel(user_thread);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Provide a server and a client name...");
        exit(0);
    }

    //names
    char server[MAXNAME];
    char client[MAXNAME];
    sprintf(server, "%s.fifo", argv[1]);
    sprintf(client, "%s>>", argv[2]);

    //init io
    simpio_set_prompt(&simpio, client); // set the prompt
    simpio_reset(&simpio); // initialize io
    simpio_noncanonical_terminal_mode(); // set the terminal into a compatible mode

    //fifos
    sprintf(to_fifo_name, "%i.to.fifo", getpid());
    sprintf(from_fifo_name, "%i.from.fifo", getpid());
    mkfifo(to_fifo_name, DEFAULT_PERMS);
    to_fifo_fd = open(to_fifo_name, O_RDWR);
    mkfifo(from_fifo_name, DEFAULT_PERMS);
    from_fifo_fd = open(from_fifo_name, O_RDWR);

    //join request
    join_t req;
    strcpy(req.name, client);
    strcpy(req.to_client_fname, from_fifo_name);
    strcpy(req.to_server_fname, to_fifo_name);
    //and then make the request
    int join_fd = open(server, O_RDWR);
    write(join_fd, &req, sizeof(join_t));
    close(join_fd);

    //start a user thread, read input
    pthread_create(&user_thread, NULL, user_thread_func, NULL);
    //start a server thread, listen to server
    pthread_create(&server_thread, NULL, server_thread_func, NULL);

    //wait for threads
    pthread_join(user_thread, NULL);
    pthread_join(server_thread, NULL);

    simpio_reset_terminal_mode();
}
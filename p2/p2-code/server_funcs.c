#include "blather.h"

// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash
client_t *server_get_client(server_t *server, int idx) {
    return &server->client[idx];
}

// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd._
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.
void server_start(server_t *server, char *server_name, int perms) {
    char fifo_name[MAXNAME];
    memset(fifo_name, '\0', MAXNAME);

    sprintf(fifo_name, "%s.fifo", server_name);
    remove(fifo_name);
    mkfifo(fifo_name, perms);
    server->join_fd = open(fifo_name, O_RDWR);
    strcpy(server->server_name, server_name);
    server->time_sec = 0;

    //Log feature 
    //server->log_fd = open(sprintf("%s.log", server_name), O_RDWR | O_CREAT);
}

// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.
void server_shutdown(server_t *server) {
    close(server->join_fd);
    unlink(server->server_name);
    mesg_t shutdown;
    memset(&shutdown, '\0', sizeof(mesg_t));
    shutdown.kind = BL_SHUTDOWN;
    server_broadcast(server, &shutdown);
    for(int i = 0; i < server->n_clients; i++) {
        server_remove_client(server, i);
    }
}

// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).
int server_add_client(server_t *server, join_t *join) {
    if (server->n_clients == MAXCLIENTS) return 1;

    client_t newClient;
    strcpy(newClient.name, join->name); //join name to new client

    //to client fd and name saved
    strcpy(newClient.to_client_fname, join->to_client_fname);
    newClient.to_client_fd = open(join->to_client_fname, O_RDWR);
    
    //to server fd and name saved
    strcpy(newClient.to_server_fname, join->to_server_fname);
    newClient.to_server_fd = open(join->to_server_fname, O_RDWR);

    //initialize data_ready and contact time
    newClient.last_contact_time = -1;
    newClient.data_ready = 0;

    //push client to client array, increment counter
    server->client[server->n_clients] = newClient;
    server->n_clients++;

    return 0;
}

// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// preserving their order in the array; decreases n_clients.
int server_remove_client(server_t *server, int idx) {
    close(server->client[idx].to_client_fd);
    close(server->client[idx].to_server_fd);
    unlink(server->client[idx].to_client_fname);
    unlink(server->client[idx].to_server_fname);

    for (int i = idx; i < server->n_clients; i++) {
        if (server->client[i + 1].name == NULL) break; //will this segfault?
        server->client[i] = server->client[i+1];
    }
    server->n_clients--;
    return idx;
}

// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.
int server_broadcast(server_t *server, mesg_t *mesg) {
    for (int i = 0; i < server->n_clients; i++) {
        write(server->client[i].to_client_fd, mesg, sizeof(*mesg));
    }
    return mesg->kind;
}

// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the poll() system call to efficiently determine
// which sources are ready.
void server_check_sources(server_t *server) {
    printf("Checking sources...\n");
    fflush(stdout);
    struct pollfd pfds[server->n_clients + 1];
    memset(pfds, '\0', sizeof(struct pollfd) * (server->n_clients + 1));

    for (int i = 0; i < server->n_clients; i++) {
        pfds[i].fd = server->client[i].to_server_fd;
        pfds[i].events = POLLIN;
    }
    //tack the join fd on there
    pfds[server->n_clients].fd = server->join_fd;
    pfds[server->n_clients].events = POLLIN;

    poll(pfds, server->n_clients + 1, -1); //THIS WILL BLOCK until data ready
    printf("Poll exited\n");
    fflush(stdout);

    //revent field is nonzero if an event happened
    for (int j = 0; j < server->n_clients; j++) {
        if (pfds[j].revents & POLLIN) {
            server->client[j].data_ready = 1;
            printf("Data ready at client %i\n", j);
            fflush(stdout);
        }
    }
    if (pfds[server->n_clients].revents & POLLIN) {
        server->join_ready = 1;
        printf("Join ready");
        fflush(stdout);
    }
}

// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.
int server_join_ready(server_t *server) {
    return server->join_ready;
}

// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.
int server_handle_join(server_t *server) {
    printf("handing join...");
    fflush(stdout);
    join_t newJoin;
    int nread = read(server->join_fd, &newJoin, sizeof(join_t));
    server_add_client(server, &newJoin);

    mesg_t joined;
    memset(&joined, '\0', sizeof(mesg_t));
    joined.kind = BL_JOINED;
    strcpy(joined.name, newJoin.name);
    server_broadcast(server, &joined);
    server->join_ready = 0;

    return nread;
}

// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.
int server_client_ready(server_t *server, int idx) {
    return server->client[idx].data_ready;
}

// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.
int server_handle_client(server_t *server, int idx) {
    printf("handling client...");
    fflush(stdout);
    mesg_t newMesg;
    int nread = read(server->client[idx].to_server_fd, &newMesg, sizeof(mesg_t));
    if (newMesg.kind == BL_MESG || newMesg.kind == BL_DEPARTED) server_broadcast(server, &newMesg);
    server->client[idx].last_contact_time = server->time_sec; //update last contact time
    server->client[idx].data_ready = 0;
    return nread;
}

// ADVANCED: Increment the time for the server
void server_tick(server_t *server) {
    server->time_sec++;
}

// ADVANCED: Ping all clients in the server by broadcasting a ping.
void server_ping_clients(server_t *server) {
    mesg_t ping;
    memset(&ping, '\0', sizeof(mesg_t));
    ping.kind = BL_PING;
    strcpy(ping.name, "PING");
    server_broadcast(server, &ping);
}

// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.
void server_remove_disconnected(server_t *server, int disconnect_secs) {
    for (int i = 0; i < server->n_clients; i++) {
        //printf("Servertime %i - Clienttime %i > dc secs %i\n", server->time_sec, server->client[i].last_contact_time, disconnect_secs);
        if ( (server->time_sec - server->client[i].last_contact_time) > disconnect_secs ) {
            //client is dc'ed
            printf("CLIENT DC'ed\n");
            mesg_t dc;
            memset(&dc, '\0', sizeof(mesg_t));
            dc.kind = BL_DISCONNECTED;
            strcpy(dc.name, server->client[i].name);
            server_remove_client(server, i);
            server_broadcast(server, &dc); //broadcast dc message
        }
    }
}

void server_write_who(server_t *server);
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

void server_log_message(server_t *server, mesg_t *mesg);
// ADVANCED: Write the given message to the end of log file associated
// with the server.

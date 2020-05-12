#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

typedef struct {
    char name[128];
    int  ipv4[4];
} host_t;

typedef struct {
    int entries;
} header_t;

char *file_bytes;

void update_ip(int host_idx, int new_ip[]) {
    int offset = sizeof(header_t) + //header struct size
            sizeof(host_t)*host_idx;
    host_t *host = (host_t *)(file_bytes + offset);
    memcpy(host->ipv4, new_ip, sizeof(new_ip));
}

int main() {
    //open and map data file
    int fd = open("host_ips.dat", O_RDWR);
    struct stat stat_buf;
    fstat(fd, &stat_buf);
    int size = stat_buf.st_size;
    file_bytes = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //read in the header
    header_t *header = (header_t *) file_bytes;
    int entries = header->entries; //get #of entries, header could hold more info in the future.

    //DO THE WORK AND SUCH HERE
    printf("Processing hosts/addresses\n");
    //update_ip(); etc....
    //!WORK

    munmap(file_bytes, stat_buf.st_size);
    close(fd);

    return 0;
}
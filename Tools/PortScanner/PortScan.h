// This file carries all functions declaration and structure definitons
#include <netinet/in.h> // provides definitions for network-related data structures, constants, and macros

struct thread_opts {
    char host[INET_ADDRSTRLEN];
    unsigned int port, timeout, thread_id, start, end;
};

int PortScan_error(const char *s, int sock);

void *worker(void *thread_args);

int scanner(const char * host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end);

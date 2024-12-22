#include <pthread.h>    // For creating and managing threads
#include <sys/select.h> // For monitoring multiple file descriptors (used in socket operations)
#include <netdb.h>      // For network-related utilities (e.g., resolving hostnames)
#include <sys/fcntl.h>  // For setting non-blocking mode on sockets
#include <arpa/inet.h>  // For converting hostnames/IPs to network addresses
#include <unistd.h>     // For system calls like sleep() and close()

#include "ArgParse.h"   // For command-line argument parsing
#include "PortScan.h"   // Contains declarations for the port scanner

#define MAX_THREADS 500

/*
 * This function is a core of this port scanner. It scans a port specified in port parameter.
 * That parameter can be changed after passing to process second port to scan. Most important
 * part of this function is we are setting socket on non blocking and waiting if we got a 
 * permission to write on socket.
*/
int scanner(const char * host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end)
{
    // This struct has all information which is reequired to connect to target
    struct sockaddr_in address, bind_addr;
    // This struct is used in select(). It contains timeout information
    struct timeval tv;
    fd_set write_fds;
    socklen_t so_error_len;
    // The socket descriptor, error status and yes
    int sd, so_error = 1, yes = 1;

    int write_permission;

    // Wait until start flag is not enabled by main process
    while(!*start) {
        sleep(2); // Wait for 2 seconds
    }

    // Process until end flag is not set by main process
    while (!*end) {
        // Wait 2 seconds till port is 0
        while (*port == 0) {
            sleep(2);
        }

        // Fill sockaddr_in struct
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(host);
        // inet_addr() converts string to host IP to int
        address.sin_port = htons(*port);
        /* htons() returns int with data set as big endian.
        Most computers follow little endian and network devices only know big endian.*/

        // Seconds to timeout
        tv.tv_sec = timeout;
        // Microseconds to timeout
        tv.tv_usec = 0;

        FD_ZERO(&write_fds);

        so_error_len = sizeof(so_error);

        // Create a socket
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            return PortScan_error("An error has occured in socket()", 0);
        }

        // Set port as reuseable. So we may not use up all available ports.
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            return PortScan_error("An error has occured in setsockopt()", 0);
        }

        // Make our socket non-blocking. Program will not stop intil connection is made.
        if (fcntl(sd, F_SETFL, O_NONBLOCK) == -1) {
            return PortScan_error("fcntl() caused error", 1);
        }

        // Now connect() function will always return -1 as we are in non-blocking flag.
        if (connect(sd, (struct sockaddr *) &address, sizeof(address)) == -1) {
            switch (errno) {
            case EWOULDBLOCK: // Processing going on
            case EINPROGRESS: // Connection in progress
                break;
            
            default: // We want to give an error in every other case
                return PortScan_error("An error has occured in connect()", sd);
            }
        }

        FD_SET(sd, &write_fds);

        // Waiting for time when we can write on socket or timeout occurs.
        if ((write_permission = select(sd + 1, NULL, &write_fds, NULL, &tv)) == -1) {
            return PortScan_error("An error has occured in select()", sd);
        }

        // If we got write permission
        if (write_permission) {
            if (getsockopt(sd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len) != -1) {
                if (so_error == 0) {
                    printf("%d OPEN\n", *port);
                }
            }
        }

        // Set port to 0. so we do not process one port again and again
        *port = 0;
    }
}

/*
 * A worket function or thread function that will run beside main. This will be responsible
 * for scanning ports passed by main function to a thread
*/
void *worker(void *thread_opts)
{
    // Create pointer to struct which carries all options passed by main
    struct thread_opts *opts;

    // Now opt will point to thread_opt passed by main
    opts = thread_opts;

    // Call a core function will do entire work of scanning
    scanner(opts->host, &opts->port, opts->timeout, &opts->start, &opts->end);

    // Exit current thread
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct arguments *user_args;

    user_args = parse_args(argc, argv);

    // If the user specified -v then print version and exit
    if (user_args->version) {
        printf("Scanner v0.1");
        exit(0);
    }

    // If the user do ot specify the host then print error and exit
    if (strlen(user_args->host) == 0) {
        PortScan_error("[-] Please specify the host\n", 1);
    }

    struct hostent * target;

    // Resolve hostname
    target = gethostbyname(user_args->host);

    // Clear out space
    bzero(user_args->host, sizeof(user_args->host));
	// Copy to struct with typecasting
	strcpy(user_args->host , inet_ntoa(*( (struct in_addr *)target->h_addr_list[0] )));
	printf("Scanning %s\n", user_args->host);


    int thread_id;
    pthread_t threads[MAX_THREADS];
    struct thread_opts opts[MAX_THREADS];
    unsigned int port_scan = 1;

    for (thread_id = 0; thread_id < MAX_THREADS; thread_id++) {
        opts[thread_id].start = 0;
        opts[thread_id].end = 0;
        opts[thread_id].port = 0;
        opts[thread_id].timeout = user_args->timeout;
        opts[thread_id].thread_id = thread_id;
        strncpy(opts[thread_id].host, user_args->host, (size_t)INET_ADDRSTRLEN); // Set target host

        // Create threads
        if (pthread_create(&threads[thread_id], NULL, worker, (void *) &opts[thread_id])) {
            #ifdef DEBUGGING
                perror("pthred_create() error")
            #endif
            return EXIT_FAILURE;
        }
    }

    thread_id = 0;
    printf("--> Created %d threads.\n", MAX_THREADS);

    // Loop till over all ports are scanned
    while (port_scan < 65535) {
        // Iterate through all threads
        for (int i = 0; i < MAX_THREADS; i++) {
            if (opts[i].port == 0) {
                opts[i].port = port_scan; // giving port to each thread to scan
                port_scan++;
                opts[i].start = 1;
            }
        }
    }

    /* 
	 * We can use any other approach to ensure all threads are exited but
	 * in our case we are sure that no thread can run more than user_args->timeout + 1.
	 * Still we are doubling time.
	 */
	sleep(user_args->timeout + user_args->timeout); // ensure all threads had done their work
}

int PortScan_error(const char *s, int sock)
{
    #ifdef DEBUGGING
    perror(s);
    #endif
    if (sock) {
        close(sock);
    }
    return 0;
}
// This file contain functions and structures responsible for parsing arguments.

#include <argp.h>   // Used for command-line argument parsing
#include <stdlib.h> // Provides functions for Memory Management, Conversion, Process Control, and Random Numbers
#include <error.h>  // Provides functions to handle and report errors in a standardized way
#include <string.h> // Provides functions to manipulate C-style strings and memory blocks

struct arguments
{
    char host[INET_ADDRSTRLEN]; // host-name or IP to scan
    int timeout; // timeout for each port
    int version; // a flag for "-v" to check version
    char file_to_output[30]; // output file
};

struct argp_option options[] = {
    {"host", 'h', "HOST", 0, "Target host to scan"},
    {"timeout", 't', "SECONDS", 0, "Speed of scanning aka seconds of timeout"},
    {"output", 'o', "FILE", 0, "Outputs to file instead of standard output"},
    {"version", 'v', 0, 0, "Prints version and exit"},
    { 0 }
};

char doc[] = "This is a simple port scanner that intend to show some logic behind port scanning.";

// A description og the arguments we accept
char args_doc[] = "";

// Keys for options without short-options.
#define OPT_ABORT 1 // Abort

error_t parse_opt(int key, char *arg, struct argp_state *state) 
{
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key)
    {
        case 'h':
            strncpy(arguments->host, arg, (size_t) INET_ADDRSTRLEN);
            break;
        case 't':
            arguments->timeout = atoi(arg);
            break;
        case 'o':
            strncpy(arguments->file_to_output, arg, 30);
            break;
        case 'v':
            arguments->version = 1;
            break;
        
        default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = { options, parse_opt, args_doc, doc };

struct arguments *parse_args(int argc, char *argv[])
{
    static struct arguments args;
    argp_parse (&argp, argc, argv, 0, 0, &args);
    return &args;
}
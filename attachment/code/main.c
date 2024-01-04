#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cache.h"
#include "main.h"

static FILE *traceFile;

int main(int argc, char **argv) {
    parse_args(argc, argv);
    init_cache();
    play_trace(traceFile);
    print_stats();
}

void parse_args(int argc, char **argv) {
    if (argc < 2) {
        printf("usage:  sim <options> <trace file>\n");
        exit(-1);
    }

    /* parse the command line arguments */
    for (int i = 0; i < argc; i++) // -h: output help message, then exit
        if (!strcmp(argv[i], "-h")) {
            printf("\t-h:  \t\tthis message\n\n");
            printf("\t-bs <bs>: \tset cache block size to <bs>\n");
            printf("\t-us <us>: \tset unified cache size to <us>\n");
            printf("\t-is <is>: \tset instruction cache size to <is>\n");
            printf("\t-ds <ds>: \tset data cache size to <ds>\n");
            printf("\t-a <a>: \tset cache associativity to <a>\n");
            printf("\t-wb: \t\tset write policy to write back\n");
            printf("\t-wt: \t\tset write policy to write through\n");
            printf("\t-wa: \t\tset allocation policy to write allocate\n");
            printf("\t-nw: \t\tset allocation policy to no write allocate\n");
            exit(0);
        }

    int arg_index = 1, value;
    while (arg_index != argc - 1) { /* set the cache simulator parameters */

        if (!strcmp(argv[arg_index], "-bs")) { // -bs <bs>: set cache block size to <bs>
            value = atoi(argv[arg_index + 1]);
            set_cache_param(CACHE_PARAM_BLOCK_SIZE, value);
            arg_index += 2;
            continue;
        }

        if (!strcmp(argv[arg_index], "-us")) { // -us <us>: set unified cache size to <us>
            value = atoi(argv[arg_index + 1]);
            set_cache_param(CACHE_PARAM_USIZE, value);
            arg_index += 2;
            continue;
        }

        if (!strcmp(argv[arg_index], "-is")) { // -is <is>: set instruction cache size to <is>
            value = atoi(argv[arg_index + 1]);
            set_cache_param(CACHE_PARAM_ISIZE, value);
            arg_index += 2;
            continue;
        }

        if (!strcmp(argv[arg_index], "-ds")) { // -ds <ds>: set data cache size to <ds>
            value = atoi(argv[arg_index + 1]);
            set_cache_param(CACHE_PARAM_DSIZE, value);
            arg_index += 2;
            continue;
        }

        if (!strcmp(argv[arg_index], "-a")) { // -a <a>: set cache associativity to <a>
            value = atoi(argv[arg_index + 1]);
            set_cache_param(CACHE_PARAM_ASSOC, value);
            arg_index += 2;
            continue;
        }

        if (!strcmp(argv[arg_index], "-wb")) { // -wb: set write policy to write back
            set_cache_param(CACHE_PARAM_WRITEBACK, value);
            arg_index += 1;
            continue;
        }

        if (!strcmp(argv[arg_index], "-wt")) { // -wt: set write policy to write through
            set_cache_param(CACHE_PARAM_WRITETHROUGH, value);
            arg_index += 1;
            continue;
        }

        if (!strcmp(argv[arg_index], "-wa")) { // -wa: set allocation policy to write allocate
            set_cache_param(CACHE_PARAM_WRITEALLOC, value);
            arg_index += 1;
            continue;
        }

        if (!strcmp(argv[arg_index], "-nw")) { // -nw: set allocation policy to no write allocate
            set_cache_param(CACHE_PARAM_NOWRITEALLOC, value);
            arg_index += 1;
            continue;
        }

        printf("error:  unrecognized flag %s\n", argv[arg_index]);
        exit(-1);
    }

    dump_settings();

    /* open the trace file */
    traceFile = fopen(argv[arg_index], "r");
}

void play_trace(FILE *inFile) {
    unsigned addr, access_type;
    int num_inst = 0;

    while (read_trace_element(inFile, &access_type, &addr)) {
        switch (access_type) {
        case TRACE_DATA_LOAD:
        case TRACE_DATA_STORE:
        case TRACE_INST_LOAD:
            perform_access(addr, access_type);
            break;
        default:
            printf("skipping access, unknown type(%d)\n", access_type);
        }

        num_inst++;
        if (!(num_inst % PRINT_INTERVAL))
            printf("processed %d references\n", num_inst);
    }

    flush();
}

//return 1 if is end of file, 0 otherwise
int read_trace_element(FILE *inFile, unsigned *access_type, unsigned *addr) {
    char c;//discard all chars
    int result = fscanf(inFile, "%u %x%c", access_type, addr, &c);
    while (c != '\n') {
        result = fscanf(inFile, "%c", &c);
        if (result == EOF)
            break;
    }
    if (result != EOF)
        return (1);
    else
        return (0);
}

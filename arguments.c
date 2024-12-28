#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arguments.h"

void printHelp() {
    printf("Usage:\n");
    printf("  -x <source file>      Optional, can be repeated multiple times.\n"
                 "                        Excludes specific folder and file patterns from the adf.\n"
                 "                        This can be a file name or a relative path, for example \"images/pic.info\" .\n"
                 "\n"
                 "  -d <dst filename>     Required. Name of the destination adf.\n"
                 "\n"
                 "  -s <src folder>       Required. Source folder to be copied into the destination adf.\n"
                 );
}

void freeArguments(struct Arguments *args) {
    if ( args ) {
        if ( args->exclusions ) {
            for ( int i=0; i<args->exclusion_count; i++ ) {
                free(args->exclusions[i]);
            }
            free(args->exclusions);
        }
        free(args);
    }
}

void printError(const char *error) {
    printf(error);
    printHelp();
}

const struct Arguments *parseArguments(const int argc, const char *argv[]) {
    struct Arguments *args = malloc(sizeof(struct Arguments));
    if (!args) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    args->error = 0;
    args->exclusions = NULL;
    args->exclusion_count = 0;
    args->dst_adf = NULL;
    args->src_folder = NULL;
    args->error_message = NULL;

    args->error = true;

    // Initialize c_values
    int x_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-x") == 0) {
            if (i + 1 < argc && strchr(argv[i + 1], '=') != NULL) {
                x_count += 1;
                ++i;
            } else {
                args->error_message = "Error: -x requires an argument <source file>\n";
                return args;
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            if (i + 1 < argc) {
                if (args->dst_adf) {
                    args->error_message = "Error: -d can only appear once.\n";
                    return args;
                }
                args->dst_adf = argv[++i];
            } else {
                args->error_message = "Error: -d requires an argument <dst filename>\n";
                return args;
            }
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                if (args->src_folder) {
                    args->error_message = "Error: -s can only appear once.\n";
                    return args;
                }
                args->src_folder = argv[++i];
            } else {
                args->error_message = "Error: -s requires an argument <src folder>\n";
                return args;
            }
        } else {
            args->error_message = "Error: Unknown argument: %s\n";
            return args;
        }
    }

    // Validate required arguments
    if (!args->src_folder) {
        args->error_message = "Error: Missing required argument -s\n";
        return args;
    }
    if (!args->dst_adf) {
        args->error_message = "Error: Missing required argument -d\n";
        return args;
    }

    args->exclusions = malloc(sizeof(char *) * x_count);

    int c=0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc && strchr(argv[i + 1], '=') != NULL) {
                args->exclusions[c++] = argv[i+1];
            } else {
                return args;
            }
        }
    }
    args->exclusion_count = x_count;

    args->error = false;

    return args;
}

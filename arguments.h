#pragma once
#include <stdbool.h>

struct Arguments {
    bool error;
    const char *error_message;
    const char *dst_adf;
    const char *src_folder;
    const char **exclusions;
    int exclusion_count;
};

#ifdef __cplusplus
extern "C" {
#endif

extern const struct Arguments *parseArguments(const int argc, const char *argv[]);
extern void freeArguments(struct Arguments *args);
extern void printError(const char *error);

#ifdef __cplusplus
}
#endif
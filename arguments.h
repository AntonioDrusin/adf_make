#pragma once
#include <stdbool.h>

struct Arguments {
    bool error;
    const char *label;
    const char *error_message;
    const char *dst_adf;
    const char *src_folder;
};

#ifdef __cplusplus
extern "C" {
#endif

extern const struct Arguments *getArguments(const int argc, const char *argv[]);
extern void freeArguments(const struct Arguments *args);
extern void printError(const char *error);

#ifdef __cplusplus
}
#endif
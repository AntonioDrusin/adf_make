#pragma once
#include "adf_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int makeAdf(
    const struct AdfLibrary *adfLib,
    const char *destinationFile,
    const char **fileList,
    int fileCount);

#ifdef __cplusplus
}
#endif
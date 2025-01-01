#include "adf_lib.h"


RETCODE adfUtilCopyFile(
    const struct AdfLibrary *adfLibrary,
    struct Volume *volume,
    const char *sourceFilePath,
    char *destFilePath
) {
    const int bufferSize = 1024;
    unsigned char buffer[bufferSize];

    if (sourceFilePath == NULL || destFilePath == NULL || volume == NULL || adfLibrary == NULL) return -1;

    struct File *file = adfLibrary->adfOpenFile(volume, destFilePath, "w");
    if (file == NULL) return -1;

    FILE *inFile = fopen(sourceFilePath, "r");
    if ( !inFile) {
        adfLibrary->adfCloseFile(file);
        return RC_ERROR;
    }

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, bufferSize, inFile)) > 0) {
        RETCODE rc = adfLibrary->adfWriteFile(file, (long)bytesRead, buffer);
        if ( rc != RC_OK ) {
            fclose(inFile);
            adfLibrary->adfCloseFile(file);
            return RC_ERROR;
        }
    }

    if ( ferror(inFile) )
    {
        perror("Error reading from source file");
        fclose(inFile);
        adfLibrary->adfCloseFile(file);
        return RC_ERROR;
    }

    fclose(inFile);
    adfLibrary->adfCloseFile(file);
    return RC_OK;
}

struct AdfLibrary realAdfLibrary = {
    .adfCreateDumpDevice = adfCreateDumpDevice,
    .adfCreateFlop = adfCreateFlop,
    .adfUnMountDev = adfUnMountDev,
    .adfMount = adfMount,
    .adfUnMount = adfUnMount,
    .adfInstallBootBlock = adfInstallBootBlock,
    .adfOpenFile = adfOpenFile,
    .adfWriteFile = adfWriteFile,

    .adfUtilCopyFile = adfUtilCopyFile,
};

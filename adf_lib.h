#pragma once
#include "adflib/lib/adflib.h"

struct AdfLibrary {
    struct Device* (*adfCreateDumpDevice)(char* name, long cylinders, long heads, long sectors);
    RETCODE (*adfCreateFlop)(struct Device* device, char* label, int fsMask);
    void (*adfUnMountDev)(struct Device* device);
    void (*adfUnMount)(struct Volume *vol);
    struct Volume* (*adfMount)( struct Device *dev, int nPart, BOOL readOnly );
    RETCODE (*adfInstallBootBlock)(struct Volume *vol, unsigned char* code);
    struct File* (*adfOpenFile)(struct Volume *vol, char *name, char *mode);
    void (*adfCloseFile)(struct File *file);
    RETCODE (*adfWriteFile)(struct File *file, long size, unsigned char *data);
    // Additional functions
    RETCODE (*adfUtilCopyFile)(const struct AdfLibrary *, struct Volume *vol, const char *sourceFilePath, char *destFilePath);
};

extern struct AdfLibrary realAdfLibrary;
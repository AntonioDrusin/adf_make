#include "make_adf.h"

#define INCBIN(name, file) \
__asm__( \
".section .rodata\n" \
".global " #name "\n" \
#name ":\n" \
".incbin \"" file "\"\n" \
".global " #name "_end\n" \
#name "_end:\n" \
); \
extern unsigned char name[]; \
extern unsigned char name##_end[]

INCBIN(bootBlock, "../adflib/Boot/stdboot3.bbk");

int makeAdf(
    const struct AdfLibrary *adfLib,
    const char *destinationFile,
    const char **fileList,
    int fileCount)
{
    long rc;
    struct Device *disk = adfLib->adfCreateDumpDevice("newdev", 80, 2, 11);

    if (!disk) {
        return -1;
    }

    rc = adfLib->adfCreateFlop(disk, "empty", 0);
    if (rc != RC_OK) {
        adfLib->adfUnMountDev(disk);
        return -1;
    }

    struct Volume *volume = adfLib->adfMount(disk, 0, FALSE);
    if (volume == NULL) {
        adfLib->adfUnMountDev(disk);
        return -1;
    }

    rc = adfLib->adfInstallBootBlock(volume, bootBlock);
    if (rc != RC_OK) {
        adfLib->adfUnMount(volume);
        adfLib->adfUnMountDev(disk);
        return -1;
    }

    for (int i = 0; i < fileCount; i++) {

    }

    adfLib->adfUnMount(volume);
    adfLib->adfUnMountDev(disk);
    return 0;
}

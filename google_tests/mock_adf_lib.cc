#include "mock_adf_lib.h"

// Global or static variable to store the reference to the mock instance
static MockAdfLibrary* g_mockAdfLibrary = nullptr;

// Trampoline Functions
Device* TrampolineCreateDumpDevice(char* name, long cylinders, long heads, long sectors) {
    return g_mockAdfLibrary->adfCreateDumpDevice(name, cylinders, heads, sectors);
}

RETCODE TrampolineCreateFlop(Device* device, char* label, int fsMask) {
    return g_mockAdfLibrary->adfCreateFlop(device, label, fsMask);
}

void TrampolineUnMountDev(Device* device) {
    g_mockAdfLibrary->adfUnMountDev(device);
}

void TrampolineUnMount(Volume* vol) {
    g_mockAdfLibrary->adfUnMount(vol);
}

Volume* TrampolineMount(Device* dev, int nPart, BOOL readOnly) {
    return g_mockAdfLibrary->adfMount(dev, nPart, readOnly);
}

RETCODE TrampolineInstallBootBlock(Volume* vol, unsigned char* code) {
    return g_mockAdfLibrary->adfInstallBootBlock(vol, code);
}

File* TrampolineOpenFile(struct Volume *vol, char *name, char *mode) {
    return g_mockAdfLibrary->adfOpenFile(vol, name, mode);
}

void TrampolineCloseFile(struct File *file) {
    return g_mockAdfLibrary->adfCloseFile(file);
}

RETCODE TrampolineWriteFile(struct File *file, long size, unsigned char *data) {
    return g_mockAdfLibrary->adfWriteFile(file, size, data);
}

AdfLibrary g_lib;

// Adapter Function
AdfLibrary *CreateAdfLibraryFromMock(MockAdfLibrary& mock) {
    g_mockAdfLibrary = &mock; // Set the global mock reference

    g_lib.adfCreateDumpDevice = TrampolineCreateDumpDevice;
    g_lib.adfCreateFlop = TrampolineCreateFlop;
    g_lib.adfUnMount = TrampolineUnMount;
    g_lib.adfUnMountDev = TrampolineUnMountDev;
    g_lib.adfMount = TrampolineMount;
    g_lib.adfInstallBootBlock = TrampolineInstallBootBlock;
    g_lib.adfOpenFile = TrampolineOpenFile;
    g_lib.adfCloseFile = TrampolineCloseFile;
    g_lib.adfWriteFile = TrampolineWriteFile;

    return &g_lib;
}

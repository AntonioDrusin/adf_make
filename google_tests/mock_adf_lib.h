#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../adf_lib.h"

class MockAdfLibrary {
public:
    MOCK_METHOD(struct Device*, adfCreateDumpDevice, (char* name, long cylinders, long heads, long sectors), ());
    MOCK_METHOD(RETCODE, adfCreateFlop, (struct Device* device, char* label, int fsMask), ());
    MOCK_METHOD(void, adfUnMountDev, (struct Device* device), ());
    MOCK_METHOD(void, adfUnMount, (struct Volume* vol), ());
    MOCK_METHOD(struct Volume*, adfMount, (struct Device* dev, int nPart, BOOL readOnly), ());
    MOCK_METHOD(RETCODE, adfInstallBootBlock, (struct Volume* vol, unsigned char* code), ());
    MOCK_METHOD(struct File*, adfOpenFile, (struct Volume *vol, char *name, char *mode), ());
    MOCK_METHOD(void, adfCloseFile, (struct File *file), ());
    MOCK_METHOD(RETCODE, adfWriteFile, (struct File *file, long size, unsigned char *data), ());
};

extern "C" {
AdfLibrary *CreateAdfLibraryFromMock(MockAdfLibrary &mock);
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock_adf_lib.h"
#include "../make_adf.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::DoAll;


TEST(MakeAdfTest, SuccessTest) {
    MockAdfLibrary mockAdfLib;
    Device mockDevice{};
    Volume mockVolume{};

    // Mock adfCreateDumpDevice to return a mock device
    EXPECT_CALL(mockAdfLib, adfCreateDumpDevice(_, _, _, _))
        .WillOnce(Return(&mockDevice));

    // Mock adfCreateFlop to return RC_OK
    EXPECT_CALL(mockAdfLib, adfCreateFlop(&mockDevice, _, _))
        .WillOnce(Return(RC_OK));

    // Mock adfMount to return a mock volume
    EXPECT_CALL(mockAdfLib, adfMount(&mockDevice, _, _))
        .WillOnce(Return(&mockVolume));

    // Mock adfInstallBootBlock to return RC_OK
    EXPECT_CALL(mockAdfLib, adfInstallBootBlock(&mockVolume, _))
        .WillOnce(Return(RC_OK));

    // Expect adfUnMount to be called for the volume
    EXPECT_CALL(mockAdfLib, adfUnMount(&mockVolume))
        .Times(1);

    // Expect adfUnMountDev to be called for the device
    EXPECT_CALL(mockAdfLib, adfUnMountDev(&mockDevice))
        .Times(1);

    // Call the function to test
    const char* fileList[] = {"file1", "file2"};
    int result = makeAdf(CreateAdfLibraryFromMock(mockAdfLib),
                         "destination.adf", fileList, 2);

    // Assert the result
    EXPECT_EQ(result, 0);
}

TEST(MakeAdfTest, OnFailedFloppy_UnmountsDevice) {
    MockAdfLibrary mockAdfLib;
    Device mockDevice{};

    // Mock adfCreateDumpDevice to return a mock device
    EXPECT_CALL(mockAdfLib, adfCreateDumpDevice(_, _, _, _))
        .WillOnce(Return(&mockDevice));

    // Mock adfCreateFlop to return RC_OK
    EXPECT_CALL(mockAdfLib, adfCreateFlop(&mockDevice, _, _))
        .WillOnce(Return(!RC_OK));


    // Expect adfUnMountDev to be called for the device
    EXPECT_CALL(mockAdfLib, adfUnMountDev(&mockDevice))
        .Times(1);

    // Call the function to test
    const char* fileList[] = {"file1", "file2"};
    int result = makeAdf(CreateAdfLibraryFromMock(mockAdfLib),
                         "destination.adf", fileList, 2);

    // Assert the result
    EXPECT_EQ(result, -1);
}

TEST(MakeAdfTest, OnFailedBootInstall_UnmountsDeviceAndVolume) {
    MockAdfLibrary mockAdfLib;
    Device mockDevice{};
    Volume mockVolume{};

    // Mock adfCreateDumpDevice to return a mock device
    EXPECT_CALL(mockAdfLib, adfCreateDumpDevice(_, _, _, _))
        .WillOnce(Return(&mockDevice));

    // Mock adfCreateFlop to return RC_OK
    // Mock adfCreateFlop to return RC_OK
    EXPECT_CALL(mockAdfLib, adfCreateFlop(&mockDevice, _, _))
        .WillOnce(Return(RC_OK));

    // Mock adfMount to return a mock volume
    EXPECT_CALL(mockAdfLib, adfMount(&mockDevice, _, _))
        .WillOnce(Return(&mockVolume));

    // Mock adfInstallBootBlock to return RC_OK
    EXPECT_CALL(mockAdfLib, adfInstallBootBlock(&mockVolume, _))
        .WillOnce(Return(!RC_OK));

    // Expect adfUnMount to be called for the volume
    EXPECT_CALL(mockAdfLib, adfUnMount(&mockVolume))
        .Times(1);

    // Call the function to test
    const char* fileList[] = {"file1", "file2"};
    int result = makeAdf(CreateAdfLibraryFromMock(mockAdfLib),
                         "destination.adf", fileList, 2);

    // Assert the result
    EXPECT_EQ(result, -1);
}

TEST(MakeAdfTest, FailureOnCreateDumpDevice) {
    MockAdfLibrary mockAdfLib;

    // Mock adfCreateDumpDevice to return NULL
    EXPECT_CALL(mockAdfLib, adfCreateDumpDevice(_, _, _, _))
        .WillOnce(Return(nullptr));

    // Call the function to test
    const char* fileList[] = {"file1", "file2"};
    int result = makeAdf(CreateAdfLibraryFromMock(mockAdfLib),
                         "destination.adf", fileList, 2);

    // Assert the result
    EXPECT_EQ(result, -1);
}

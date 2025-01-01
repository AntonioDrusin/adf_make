#include <fstream>
#include <filesystem>
#include <cstring>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock_adf_lib.h"
#include "../adf_lib.h"
#include "../make_adf.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::DoAll;

namespace fs = std::filesystem;

static uint32_t calculateCrc(const std::string& data) {
    uint32_t crc = 0xFFFFFFFF;
    for (unsigned char byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

static uint32_t calculateCrc(const unsigned char* data, size_t len) {
    std::string strData(reinterpret_cast<const char*>(data), len);
    return calculateCrc(strData);
}

static uint32_t writeTestData(const std::string& filepath, size_t size_in_bytes) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    std::string data_pattern = "TESTPATTERN"; // 11 bytes
    size_t pattern_size = data_pattern.size();
    size_t total_written = 0;

    // Write repeating pattern to fill up the file
    while (total_written + pattern_size < size_in_bytes) {
        file.write(data_pattern.data(), pattern_size);
        total_written += pattern_size;
    }

    // Fill remaining bytes
    if (total_written < size_in_bytes) {
        size_t remaining = size_in_bytes - total_written;
        file.write(data_pattern.data(), remaining);
        total_written += remaining;
    }
    file.close();

    // Calculate CRC32 for the written data
    std::string final_data(size_in_bytes, '\0');
    std::ifstream verify_file(filepath, std::ios::binary);
    verify_file.read(&final_data[0], size_in_bytes);
    uint32_t crc = calculateCrc(final_data);
    verify_file.close();

    return crc;
}


static void setupDirectory(std::string& base_path) {
    base_path = (std::filesystem::temp_directory_path() / "test_dir").string();
    // Create test directory structure
    fs::create_directories(base_path + "/subdir1");
    fs::create_directories(base_path + "/subdir2");

    std::ofstream(base_path + "/file1.txt").close();
    std::ofstream(base_path + "/file2.txt").close();
    std::ofstream(base_path + "/subdir1/file3.txt").close();
    std::ofstream(base_path + "/subdir2/file4.txt").close();
}

static void cleanupDirectory(const std::string& base_path) {
    fs::remove_all(base_path); // Remove all files and directories
}

TEST(AdfLibraryCopyFile, WhenNoLibrary_ReturnsError) {
    Volume mockVolume{};

    char *destinationPath = strdup("destination");
    int result = realAdfLibrary.adfUtilCopyFile(nullptr, &mockVolume, "source", destinationPath);

    EXPECT_EQ(result, RC_ERROR);

    free(destinationPath);
}

TEST(AdfLibraryCopyFile, WhenAllArgumentsSpecifiedButNoSourceFileExists_Fails) {
    std::string test_dir;
    setupDirectory(test_dir);
    MockAdfLibrary mockAdfLib;
    Volume mockVolume{};
    File mockFile{};

    EXPECT_CALL(mockAdfLib, adfOpenFile(&mockVolume, _, _))
            .WillOnce(Return(&mockFile));

    EXPECT_CALL(mockAdfLib, adfCloseFile(&mockFile ))
            .Times(1);

    // Call the function to test
    struct AdfLibrary *adfLibrary = CreateAdfLibraryFromMock(mockAdfLib);
    char *destinationPath = strdup("destination");

    auto sourceFileName = test_dir + "/test_dir/file1.txt";
    int result = realAdfLibrary.adfUtilCopyFile(adfLibrary, &mockVolume, sourceFileName.c_str() , destinationPath);

    // Assert the result
    EXPECT_EQ(result, RC_ERROR);
    free(destinationPath);
    cleanupDirectory(test_dir);
}

TEST(AdfLibraryCopyFile, WhenAllArgumentsSpecified_Succeeds) {
    std::string test_dir;
    setupDirectory(test_dir);

    MockAdfLibrary mockAdfLib;
    Volume mockVolume{};
    File mockFile{};

    EXPECT_CALL(mockAdfLib, adfOpenFile(&mockVolume, _, _))
            .WillOnce(Return(&mockFile));

    EXPECT_CALL(mockAdfLib, adfCloseFile(&mockFile ))
            .Times(1);

    // Call the function to test
    struct AdfLibrary *adfLibrary = CreateAdfLibraryFromMock(mockAdfLib);
    char *destinationPath = strdup("destination");
    auto sourceFileName = test_dir + "/file1.txt";
    int result = realAdfLibrary.adfUtilCopyFile(adfLibrary, &mockVolume, sourceFileName.c_str(), destinationPath);

    // Assert the result
    EXPECT_EQ(result, RC_OK);
    free(destinationPath);
    cleanupDirectory(test_dir);
}

TEST(AdfLibraryCopyFile, WhenAllArgumentsSpecified_CopiesFile) {
    constexpr auto fileSize = 65890;

    std::string test_dir;
    setupDirectory(test_dir);

    MockAdfLibrary mockAdfLib;
    Volume mockVolume{};
    File mockFile{};
    unsigned char fileData[fileSize];
    size_t curPos = 0;
    bool overwrite = false;


    auto actualCrc = writeTestData(test_dir+"/test_file.txt", fileSize);

    EXPECT_CALL(mockAdfLib, adfOpenFile(&mockVolume, _, _))
            .WillOnce(Return(&mockFile));

    EXPECT_CALL(mockAdfLib, adfCloseFile(&mockFile ))
            .Times(1);

    EXPECT_CALL(mockAdfLib, adfWriteFile(&mockFile, _, _))
        .WillRepeatedly(testing::Invoke([&overwrite, &curPos, &fileData](void* file, size_t size, unsigned char* data) {
            if (size + curPos <= fileSize) {
                std::memcpy(fileData+curPos, data, size); // Copy the data to the existing array
                curPos += size;
            }
            else {
                overwrite = true;
            }
            return RC_OK;
        }));

    // Call the function to test
    AdfLibrary *adfLibrary = CreateAdfLibraryFromMock(mockAdfLib);
    char *destinationPath = strdup("destination");
    auto sourceFileName = test_dir + "/test_file.txt";
    int result = realAdfLibrary.adfUtilCopyFile(adfLibrary, &mockVolume, sourceFileName.c_str(), destinationPath);

    // Assert the result
    EXPECT_EQ(result, RC_OK);
    EXPECT_FALSE(overwrite);

    // Assert the file was actually copied
    auto resultCrc = calculateCrc(fileData, fileSize);
    EXPECT_EQ(resultCrc, actualCrc);

    free(destinationPath);
    cleanupDirectory(test_dir);
}


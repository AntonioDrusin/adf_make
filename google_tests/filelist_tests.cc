#include <fstream>
#include <filesystem>
#include <iterator>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "../list_files.h"

namespace fs = std::filesystem;

void setup_test_directory(std::string& base_path) {
    base_path = (std::filesystem::temp_directory_path() / "test_dir").string();
    // Create test directory structure
    fs::create_directories(base_path + "/subdir1");
    fs::create_directories(base_path + "/subdir2");

    std::ofstream(base_path + "/file1.txt").close();
    std::ofstream(base_path + "/file2.txt").close();
    std::ofstream(base_path + "/subdir1/file3.txt").close();
    std::ofstream(base_path + "/subdir2/file4.txt").close();
}

void cleanup_test_directory(const std::string& base_path) {
    fs::remove_all(base_path); // Remove all files and directories
}

std::vector<std::string> convert_file_array_to_vector(char **files, int fileCount) {
    std::vector<std::string> file_list;
    for (int i = 0; i < fileCount; ++i) {
        file_list.emplace_back(files[i]); // Convert each C string to std::string
    }
    return file_list;
}

TEST(FileSystemTest, TraversesDirectoryCorrectly) {
    std::string test_dir;
    int fileCount;

    // Setup test environment
    setup_test_directory(test_dir);

    // Call your function to get the list of files
    char **files = get_file_list(test_dir.c_str(), &fileCount);

    // Validate results
    EXPECT_EQ(fileCount, 4);
    EXPECT_TRUE(files != NULL);
    std::vector<std::string> file_list = convert_file_array_to_vector(files, fileCount);
    EXPECT_THAT(file_list, ::testing::UnorderedElementsAre(
        "file1.txt",
        "file2.txt",
        "subdir1/file3.txt",
        "subdir2/file4.txt"
    ));

    // Cleanup test environment
    cleanup_test_directory(test_dir);
}
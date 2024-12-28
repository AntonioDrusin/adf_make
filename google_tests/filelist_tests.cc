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

void setup_empty_test_directory(std::string& base_path) {
    base_path = (std::filesystem::temp_directory_path() / "test_dir_empty").string();
    // Create test directory structure
    fs::create_directories(base_path + "/subdir1");
    fs::create_directories(base_path + "/subdir2");
}

void cleanup_test_directory(const std::string& base_path) {
    fs::remove_all(base_path); // Remove all files and directories
}

std::vector<std::string> convert_file_array_to_vector(const char **files, int fileCount) {
    std::vector<std::string> file_list;
    for (int i = 0; i < fileCount; ++i) {
        file_list.emplace_back(files[i]); // Convert each C string to std::string
    }
    return file_list;
}

TEST(FileSystemTest, WhenFolderContainsFilesAndSubfolders_TraversesDirectoryCorrectly) {
    std::string test_dir;
    int fileCount;

    // Setup test environment
    setup_test_directory(test_dir);

    // Call your function to get the list of files
    FileList *list = getFileList(test_dir.c_str());

    // Validate results
    EXPECT_EQ(list->files_count, 4);
    EXPECT_TRUE(list->files != NULL);
    std::vector<std::string> file_list = convert_file_array_to_vector(list->files, list->files_count);
    EXPECT_THAT(file_list, ::testing::UnorderedElementsAre(
        "file1.txt",
        "file2.txt",
        "subdir1/file3.txt",
        "subdir2/file4.txt"
    ));

    // Cleanup test environment
    cleanup_test_directory(test_dir);
    freeFileList(list);
}

TEST(FileSystemTest, WhenFolderIsEmpty_ReturnsEmptyList) {
    std::string test_dir;
    int fileCount;

    // Setup test environment
    setup_empty_test_directory(test_dir);

    // Call your function to get the list of files
    FileList *list = getFileList(test_dir.c_str());

    // Validate results
    EXPECT_EQ(list->files_count, 0);
    EXPECT_TRUE(list->files != NULL);
    std::vector<std::string> file_list = convert_file_array_to_vector(list->files, list->files_count);
    EXPECT_THAT(file_list, ::testing::UnorderedElementsAre(    ));

    // Cleanup test environment
    cleanup_test_directory(test_dir);
    freeFileList(list);
}
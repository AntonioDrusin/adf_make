#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

const char* remove_starting_path(const char* full_path, const char* base_path) {
    size_t base_len = strlen(base_path);
    const char* match = strstr(full_path, base_path);

    if (match == full_path) {
        full_path += base_len; // Skip the base path
    }
    if ( full_path[0] == '/') full_path++;

    return full_path; // Return original path if base path not found
}

char **recurse_get_file_list(const char* base_path, const char *path, int *file_count) {
    struct dirent *entry;

    DIR *dp = opendir(path);
    if (dp == NULL) {
        perror("opendir");
        return NULL;
    }

    char **files = NULL; // Dynamically allocated list of file paths
    *file_count = 0;     // Initialize file count

    while ((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat path_stat;
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &path_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            // If it's a directory, recursively call list_files
            int subfile_count = 0;
            char **subfiles = recurse_get_file_list(base_path, full_path, &subfile_count);

            if (subfiles != NULL) {
                // Append subdirectory files to the main list
                files = realloc(files, (*file_count + subfile_count) * sizeof(char *));
                for (int i = 0; i < subfile_count; ++i) {
                    files[*file_count + i] = subfiles[i];
                }
                *file_count += subfile_count;
                free(subfiles); // Free the temporary subdirectory list pointer
            }
        } else {
            // If it's a file, add its path to the list
            files = realloc(files, (*file_count + 1) * sizeof(char *));
            files[*file_count] = strdup(remove_starting_path(full_path, base_path)); // Duplicate the file path string
            (*file_count)++;
        }
    }

    closedir(dp);

    return files;
}

char **get_file_list(const char *path, int *file_count) {
    char original_dir[PATH_MAX];

    char **results = recurse_get_file_list(path, path, file_count);


    return results;
}



// Function to free the list of file paths
void free_file_list(char **files, int file_count) {
    for (int i = 0; i < file_count; ++i) {
        free(files[i]);
    }
    free(files);
}

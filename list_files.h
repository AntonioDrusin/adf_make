#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char **get_file_list(const char *path, int *file_count);
extern void free_file_list(char **files, int file_count);

#ifdef __cplusplus
}
#endif
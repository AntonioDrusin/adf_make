#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct FileList {
    const char **files;
    int files_count;
};

extern struct FileList *getFileList(const char *path);
extern void freeFileList(struct FileList *list);

#ifdef __cplusplus
}
#endif
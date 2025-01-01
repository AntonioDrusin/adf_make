#pragma once

struct FileList {
    const char **files;
    int files_count;
};


#ifdef __cplusplus
extern "C" {
#endif

extern struct FileList *getFileList(const char *path);
extern void freeFileList(struct FileList *list);

#ifdef __cplusplus
}
#endif
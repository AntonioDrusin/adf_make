#include "adflib/lib/adflib.h"
#include "arguments.h"
#include "list_files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>



int main(const int argc, const char *argv[]) {
    BOOL quad;
    const struct Arguments* args = getArguments(argc, argv);

    // printf ("mounting the ADF");
    // adfEnvInitDefault();
    // struct Device *device = adfMountDev("d:\\dev\\Amiga\\musicdisck.adf");
    // if (device) {
    //     printf ("device is mounted\n");
    //     struct Volume *volume = adfMount(device, 0, TRUE);
    //     if ( volume ) {
    //         struct List *list, *cell;
    //         struct Entry *entry;
    //         cell = list = adfGetDirEnt(volume, volume->curDirPtr);
    //         if ( list ) {
    //             while ( cell ) {
    //                 entry = (struct Entry*)cell->content;
    //                 char *type = entry->type < 0 ? "FILE": "DIR";
    //                 printf("%s %s %ld\n", type, entry->name, entry->size);
    //                 cell = cell->next;
    //             }
    //             adfFreeDirList(list);
    //         }
    //         adfUnMount(volume);
    //     }
    //     adfUnMountDev(device);
    // }

    //list_files(args->src_folder);

    int count = args->exclusion_count;
    freeArguments(args);
    return count;
}


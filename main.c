#include "adflib/lib/adflib.h"
#include "arguments.h"

int main(const int argc, const char *argv[]) {
    BOOL quad;
    const struct Arguments* args = parseArguments(argc, argv);

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
    int count = args->exclusion_count;
    freeArguments(args);
    return count;
}


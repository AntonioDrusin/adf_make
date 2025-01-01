#include "adflib/lib/adflib.h"
#include "arguments.h"


int main(const int argc, const char *argv[]) {
    BOOL quad;
    const struct Arguments* args = getArguments(argc, argv);

    freeArguments(args);
    return 0;
}


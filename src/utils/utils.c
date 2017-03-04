#include "utils.h"

void memory_error(char *msg)
{
    fprintf(stderr, KRED"Error during memory reserve attempt.\n"KNRM);
    fprintf(stderr, KRED"Msg: %s\n",msg);
    fprintf(stderr, KRED"Exit Program due to unmanaged error.\n"KNRM);

    exit(1);
}


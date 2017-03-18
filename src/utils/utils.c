#include "utils.h"

static bool _verbosity_level_protected = false;


void memory_error(char *msg)
{
    if ( true == is_verbose() ) fprintf(stderr, KRED"Error during memory reserve attempt.\n"KNRM);
    if ( true == is_verbose() ) fprintf(stderr, KRED"Msg: %s\n",msg);
    if ( true == is_verbose() ) fprintf(stderr, KRED"Exit Program due to unmanaged error.\n"KNRM);
    exit(EXIT_FAILURE);
}

void verbose(bool verbosity){
    _verbosity_level_protected = verbosity;
}

bool is_verbose(){
    return _verbosity_level_protected;
}


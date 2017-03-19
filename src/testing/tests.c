#include <stdio.h>
#include <stdlib.h>
/* #include "message_struct_test.h" */
#include "greatest.h"

SUITE_EXTERN(msg_struct);

GREATEST_MAIN_DEFS();

int main(int argc, char *argv[]) {
    GREATEST_MAIN_BEGIN();      /* init & parse command-line args */
    RUN_SUITE(msg_struct);
    GREATEST_MAIN_END();        /* display results */

    return EXIT_SUCCESS;
}


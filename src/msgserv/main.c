#include <stdio.h>
#include <stdlib.h>


int usage(char* name) {
    if (name == NULL) {
        return 1;
    }
    fprintf(stdout, "Example Usage: %s [-i siip] [-p sipt]\n", name);
    fprintf(stdout, "Arguments:\n"
            "\t-i\t\t[server ip]\n"
            "\t-p\t\t[server port]\n");
    return 0;
}


int main(int argc, char *argv[]) {
    if (usage(argv[0]) == 1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

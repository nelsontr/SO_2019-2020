#include "../tecnicofs-api-constants.h"
#include "../tecnicofs-client-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


int main(int argc, char** argv) {
     if (argc != 2) {
        printf("Usage: %s sock_path\n", argv[0]);
        exit(0);
    }
    int fd = -1;
    assert(tfsMount(argv[1]) == 0);
    assert(tfsCreate("a", RW, READ) == 0);

    printf("Test: delete file success\n");
    assert(tfsDelete("a") == 0);
    
    printf("Test: delete file that does not exist\n");
    assert(tfsDelete("b") == TECNICOFS_ERROR_FILE_NOT_FOUND);
    assert(tfsUnmount() == 0);
    return 0;
}
#include "../tecnicofs-api-constants.h"
#include "../tecnicofs-client-api.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(int argc, char* argv[]) {
    assert(tfsMount(argv[1]) == 0);
    printf("Test: create file sucess\n");
    assert(tfsCreate("abc", RW, READ) == 0);
    sleep(60);
    assert(tfsUnmount() == 0);
    exit( 0);
}
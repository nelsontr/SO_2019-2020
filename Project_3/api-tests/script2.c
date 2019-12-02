#include "../tecnicofs-api-constants.h"
#include "../tecnicofs-client-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

int main(int argc, char* argv[]) {
   int fd = -1;
    char readBuffer[4] = {0};
   assert(tfsMount(argv[1]) == 0);
   assert((fd = tfsOpen("abc", WRITE)) == 0);
   assert(tfsRead(fd,readBuffer,5) == TECNICOFS_ERROR_INVALID_MODE);
   assert(tfsRename("abc","bc") == TECNICOFS_ERROR_PERMISSION_DENIED);
   assert(tfsClose(fd) == 0);
   assert(tfsUnmount()==0);
   exit(0);
}
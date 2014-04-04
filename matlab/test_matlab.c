#include "ttag.h"
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
int main() {
    tt_buf* buffer = tt_create(0,50);
    uint64_t data[16] = {10,11,12,13,14,15,16,17,18,19,110,111,112,113,114,115};
    uint8_t channels[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    
    tt_addarray(buffer,channels,data,16);
    tt_addrunner(buffer);
    tt_setresolution(buffer,0.1);
    while (tt_runners(buffer)) {
        printf("Runners: %i\n",tt_runners(buffer));
        sleep(1);
    }
    tt_close(buffer);
    return 0;
}

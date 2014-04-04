#include "ttag.h"
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
int main() {
    int i;
    tt_buf* buffer0 = tt_create(0,50);
    tt_buf* buffer1 = tt_create(1,50);
    tt_buf* buffer=0;
    
    uint64_t timetag = 1;
    uint8_t channel = 5;
    
    tt_setchannels(buffer0,5);
    tt_setchannels(buffer1,3);
    
    for (i=0;i< 20;i++) {
        tt_add(buffer0,3,timetag++);
        tt_add(buffer1,4,timetag++);
    }
    printf("Turn on Sifter... then type 0\n");
    scanf("%i",&i);
    
    for (i=0;i< 20;i++) {
        printf("0 2 %lu\n",timetag);
        tt_add(buffer0,2,timetag++);
        printf("1 3 %lu\n",timetag);
        tt_add(buffer1,3,timetag++);
    }
    printf("1 0 %lu\n",timetag);
    tt_add(buffer1,0,timetag++);
    for (i=0;i<5;i++) {
        printf("0 2 %lu\n",timetag);
        tt_add(buffer0,2,timetag++);
        tt_add(buffer1,3,timetag++);
    }
    printf("0 0 %lu\n",timetag);
    tt_add(buffer0,0,timetag++);
    for (i=0;i<20;i++) {
        printf("0 2 %lu\n",timetag);
        tt_add(buffer0,2,timetag++);
        printf("1 3 %lu\n",timetag);
        tt_add(buffer1,3,timetag++);
    }
    printf("Ready to dump contents...");
    scanf("%i",&i);
    buffer = tt_open(2);
    for (i=0;i<tt_datanum(buffer);i++) {
        printf("%i %lu\n",(int)tt_channel(buffer,i),tt_tag(buffer,i));
    }
    tt_close(buffer);
    printf("Done...");
    scanf("%i",&i);
    tt_close(buffer0);
    tt_close(buffer1);
    return 0;
}

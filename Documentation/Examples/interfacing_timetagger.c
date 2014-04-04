#include <stdio.h>
#include <signal.h>
#include "ttag.h"
char exiting = 0; static void onexit() {exiting=1;}

int main() {
    double timetagger_resolution = 1e-5;    //Size in seconds of one time bin
    int channel_number = 8;                 //Number of channels available
    int buffernumber = tt_getNextFree();    //Get an available buffer number
    tt_buf* buffer;                         //Create the object to hold a buffer
    
    signal(SIGINT,onexit);signal(SIGTERM,onexit); //Exit cleanly on ctrl+c
    
    //Create a buffer with 100 million datapoints
    buffer = tt_create(buffernumber,100000000);
    
    //Crash the program if buffer creation failed
    if (!buffer) {printf("Failed to create buffer!\n");return 1;}
    
    //Tell the user which buffer number this is, so that they can access it
    printf("Created buffer number %i\n",buffernumber);
    
    //Set relevant parameters to the buffer
    tt_setresolution(buffer,timetagger_resolution)
    tt_setchannels(buffer,channel_number);
    
    //Here goes code to prepare the time tagger for taking data. This
    //  is specific to your hardware
    
    //Take the data
    while (!exiting) {
        while (!tt_running(buffer) && !exiting); if (exiting) break;
        
        //Start running datataking on TDC
        
        while (tt_running(buffer) && !exiting) {
            //Take a piece of data from TDC here, correct timer resets according
            //to your TDC, and use the following loop to add the data to buffer
            
            tt_writeindex(buffer)+= <AMOUNT OF TIME TAGS>
            for (int i=0; i< <AMOUNT OF TIME TAGS>;i++) {
                tt_channel(buffer,tt_datanum(buffer))=<CHANNEL NUMBER>;
                tt_tag(buffer,tt_datanum(buffer))= <CORRECTED TIME STAMP>;
                tt_datanum(buffer)++
            }
        }
        //Stop running datataking on TDC
    }
    //TDC cleanup and shutdown code goes here
    
    tt_close(buffer);   //Close the buffer
    return 0;
}


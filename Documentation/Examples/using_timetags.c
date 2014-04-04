#include <stdio.h>
#include "ttag.h"   //Includes libTTag

int main() {
    tt_buf* buffer;
    int timetagger
    double seconds_needed = 1.0;    //The seconds of data needed for analysis
    
    //Need to know the buffer number to do analysis on
    printf("Which time tagger do you want?");
    scanf("%i",&timetagger);
    
    buffer = tt_open(timetagger);
    if (!buffer) { printf("Could not open time tagger!"); return 1;}
    
    //Check if there is enough data on the buffer to start analysis
    if (tt_bin2time(buffer,tt_reference(buffer)) < seconds_needed) {
        printf("Taking more data...\n");
        
        tt_addrunner(buffer); //Tell the time tagger to start taking data
        
        //Wait until the reference time is correct. Reference time is amount of
        //  time that there actually was data being taken
        while (tt_bin2time(buffer,tt_reference(buffer)) < seconds_needed);
        
        tt_remrunner(buffer); //Tell time tagger that you don't need more data
    }
    printf("Running analysis...\n");
    
    //Here you can do what you want. This template includes a loop for all data
    //currently in the buffer, starting from the oldest datapoint
    for (uint64_t i=tt_minindex(buffer); i < tt_datanum(buffer) ; i++) {
        <THE TIME STAMP> = tt_tag(buffer,i);
        <THE CHANNEL> = tt_channel(buffer,i);
        
        //Do your magic here
    }
    
    tt_close(buffer);   //We are done, the buffer is no longer necessary
    return 0;
}


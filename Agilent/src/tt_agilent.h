/* The UIUC/NCSA license:

Copyright (c) 2014 Kwiat Quantum Information Group
All rights reserved.

Developed by:	Kwiat Quantum Information Group
University of Illinois, Urbana-Champaign (UIUC)
http://research.physics.illinois.edu/QI/Photonics/

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal with the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimers
in the documentation and/or other materials provided with the distribution.

Neither the names of Kwiat Quantum Information Group, UIUC, nor the names of its contributors may be used to endorse
or promote products derived from this Software without specific prior written permission.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/

/*
Simplifier for Agilent Timetagger interface
    
Basic sample code:

//Function that reads out and does stuff with data
int dataReader(AqT3DataDescriptor* taggerReadout,void* dataptr) {
    int n;
    
    //For each sample
    for (n = 0 ; n < taggerReadout->nbrSamples ; ++n)
    {
        //Decode the readout array
        ViInt32 sample = ((ViInt32*)taggerReadout->dataPtr)[n];
        ViInt32 flag = (sample & 0x80000000) >> 31;
        ViInt32 channel = (sample & 0x70000000) >> 28; 
        ViInt32 count = (sample & 0x0fffffff);
        
        //Do stuff with the data
        printf("%i %i %i %i",sample,flag,channel,count);
    }
}

int main() {
    ViSession timeTaggerID;
    int timeTaggerNumber;
    
    //Gets the number of time taggers
    timeTaggerNumber = att_getTimeTaggerAmount();
    
    //Fail if no time taggers
    if (timeTaggerNumber<=0) {
        printf("No Time Tagger found! Exiting.");
        return 1;
    }
    
    //At least one timetagger was found - so initialize the first one
    timeTaggerID = att_initializeTimeTagger(0);
    
    //Enable all channels to 1.0V threshold
    att_setAllChannels(timeTaggerID,1.0,0);
    
    //Run acquisition given a 'data reader' function
    att_runAcquisition(timeTaggerID,dataReader,NULL,NULL);
}
*/

//Includes for the timetagger's libraries
#include <AcqirisImport.h>
#include <AcqirisT3Import.h>
#include <stdint.h>

//Defines the size of the memory bank of a TC890 time tagger
//This was found to be 8MB - 4bytes by 
//setting to 8MB in AcqrsT3_configMemorySwitch, and getting corrected value.
#define TC890_MEMSIZE (8*1024*1024-4)

#define TC890_DMASIZE  2097152	//The maximum number of elements that one bank of the time tagger can hold

//--------------------------------------------------------------------
//Getting data about instruments connected to computer

//Returns the number of instruments Agilent driver detected
int att_getInstrumentNumber();

//Given an index, returns whether the device is a time tagger
int att_isTimeTagger(int instrumentIndex);

//Returns the number of time taggers recognized by the driver
int att_getTimeTaggerAmount();

//Returns the instrument index of the nth time tagger
int att_getTimeTaggerInstrumentIndex(int timeTaggerIndex);

//--------------------------------------------------------------------
//Initialization and Closing of timetaggers

//Initializes the nth time tagger. Note that this initialization will only work for the TC890! (U1051A)
ViSession att_initializeTimeTagger(int timeTaggerIndex,int commonNumber,int useref, double timeout,int memsize);

//Initialize the given instrument
ViSession att_initializeInstrument(int instrumentIndex);

//Close the instrument
void att_closeInstrument(ViSession instrumentID);

//--------------------------------------------------------------------
//Setting channels and the like

//Sets the given channelNumber to thresholdVolts
//Can also set leading or trailing edge
int att_setChannel(ViSession timeTaggerID, int channelNumber, float thresholdVolts, int edge);

//Disables the given channel number
int att_disableChannel(ViSession timeTaggerID, int channelNumber);

//Enables all of the time tagger's standard channels (ie, not common channel or veto), and sets
//all of their thresholds to the thresholdVolts. You can also set leading or trailing edge.
int att_setAllChannels(ViSession timeTaggerID, float thresholdVolts,int edge);

//Returns the number of channels the driver detected on the given instrument
int att_getInstrumentChannelNumber(ViSession instrumentID);

//--------------------------------------------------------------------
//Obtaining Data

//Given the timeTaggerID and a pointer to a a buffer
//There is also the option to send a pointer to a boolean value - while this value is not 0, data
//  acquisition will continue, when this is set to false, data acquisition will end once the acquire call ends.

int att_runAcquisition(ViSession timeTaggerID, ViInt32* buffer,uint64_t bufsize,uint64_t volatile* bufferloc,int volatile*keepRunning, int (*analysisfunction)(ViInt32*,int,void*),void* passvalue);






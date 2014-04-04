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
    Functions that simplify interface with Agilent timetagger.
    Documentation in tt_agilent.h    
*/

//Includes self
#include "tt_agilent.h"
#include "ttag.h"
//Includes for the Time Tagger

#include <stdlib.h> //The standard library
#include <stdio.h>  //Printing and error output
#include <string.h> //Sprintf necessary

//For easy debugging
/*
#ifndef NDEBUG
#   define DBG(s) s
#   define NDBG(s) 
#else
#   define DBG(s) 
#   define NDBG(s) s
#endif
*/

//Error checking macros - they simplify the code for error checking to make the actual
//  actions easily visible, without extra convolution.
#define STAT_CHKERR(s,r,...) if (s != VI_SUCCESS) { \
    fprintf(stderr,"%i:%s: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    return r; \
}
#define ASSERT(as,r) if (!(as)) { \
    fprintf(stderr,"%i:%s: Assertion '%s' failed!\n",__LINE__,__FUNCTION__, #as ); \
    return r; \
}
#define WARN(as,r,...) if (as) { \
    fprintf(stderr,"%i:%s:WARNING: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}
    
#define ERROR(r,...) fprintf(stderr,"%i:%s: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\n"); \
    return r;

#define PRINT(...) printf("> "); printf(__VA_ARGS__); printf("\n");


//Returns the number of instruments that the driver detected
int att_getInstrumentNumber() {
    ViInt32 instrumentNumber;
    ViStatus status = Acqrs_getNbrInstruments(&instrumentNumber);
    
    //Make sure that the call was successful
    STAT_CHKERR(status, 0,"Failed to get number (%X)",status);
    
    //Return the actual number
    return (int)instrumentNumber;
}

//Returns the number of channels the driver detected on the given instrument
int att_getInstrumentChannelNumber(ViSession instrumentID) {
    ViInt32 channelNumber;
    ViStatus status = Acqrs_getNbrChannels(instrumentID,&channelNumber);
    
    //Make sure that the call was successful
    STAT_CHKERR(status, 0,"Failed to get number (%X)",status);
    
    return (int)channelNumber;
}

//Given an index, returns whether the device is a time tagger
int att_isTimeTagger(int instrumentIndex) {
    ViInt32 instrumentType;
    ViStatus status;
    
    ASSERT(instrumentIndex >= 0,0);   //Make sure there are no negative numbers
    
    status = Acqrs_getDevTypeByIndex((ViInt32)instrumentIndex,&instrumentType);
    
    STAT_CHKERR(status, 0, "Device %i query failed (%X)\n",instrumentIndex,status);
    
    //AqT3 is the time tagger's instrument type
    return (instrumentType == AqT3);
}

//Returns the number of time taggers recognized by the driver
int att_getTimeTaggerAmount() {
    int i;
    int timetaggerNumber = 0;
    int totalInstrumentNumber = att_getInstrumentNumber();
    
    //Loop through the instruments to check the number of time taggers
    for (i=0; i < totalInstrumentNumber ; i++) {
        if (att_isTimeTagger(i)) timetaggerNumber++;
    }
    
    return timetaggerNumber;
}

//Returns the instrument index of the nth time tagger
int att_getTimeTaggerInstrumentIndex(int timeTaggerIndex) {
    int i;
    int totalInstrumentNumber = att_getInstrumentNumber();
    
    //Make sure the time tagger index is within the correct range
    ASSERT(timeTaggerIndex >= 0,-1);
    ASSERT(timeTaggerIndex < att_getTimeTaggerAmount(),-1);
    
    for (i=0; i < totalInstrumentNumber; i++) {
        //Check if the current instrument is a time tagger
        if (att_isTimeTagger(i)) {
            //Check if the current instrument is the one we want
            if (timeTaggerIndex<=0) {
                //If so, return the index
                return i;
            }
            timeTaggerIndex--;
        }
    }
    //Something went wrong, and the time tagger was not found.
    //This should never happen
    ERROR(-1,"Time Tagger not found - was it just disconnected?");
}

//Initialize the nth instrument
ViSession att_initializeInstrument(int instrumentIndex) {
    ViSession instrumentID;
    ViStatus status;
    
    //14 characters supports 999 devices - well out of the range of the driver
    char deviceName[14];
    ASSERT(instrumentIndex <= 999,(ViSession)0); //Nontheless, make sure the number is within range
    
    
    //A ViSession is defined as a Uint32 - so negatives are invalid
    ASSERT(instrumentIndex >= 0,(ViSession)0);   //Negative numbers are not what we want
    
    //Set up the device name
    sprintf(deviceName,"PCI::INSTR%i",instrumentIndex);
    
    //Acqrs_init - Initializes the device
    // 1) Device name (As initialized above)
    // 2) Currently has no function, so FALSE for future-proof
    // 3) Whether or not to reset the device upon initialization
    // 4&) The return value is the actual ID of the initialized instrument
    //Changed to InitWithOptions, since calibration can possibly yield better results
    //once the instrument's setup is complete (although that's doubtful)
    status = Acqrs_InitWithOptions(deviceName,VI_FALSE,VI_FALSE,"CAL=0",&instrumentID);
    
    //Check to make sure there are no errors, and that the assumptions made are correct
    STAT_CHKERR(status,(ViSession)0,"Instrument %i failed initialization (%X)",instrumentIndex,status)
    
    ASSERT(instrumentID != 0,(ViSession)0); //I am assuming that an instrumentID of 0 is invalid
    
    return instrumentID;
}

//Closes the instrument associated with the ID
void att_closeInstrument(ViSession instrumentID) {
    ViStatus status;
    
    ASSERT(instrumentID != 0,);
    
    status = Acqrs_close(instrumentID);
    
    STAT_CHKERR(status,,"Closing failed (%X)",status);
}

//Initializes the relevant channel
int att_setChannel(ViSession timeTaggerID, int channelNumber, float thresholdVolts, int edge) {
    ViStatus status;
    
    ASSERT(timeTaggerID != 0, 1);
    
    //The channelNumber can be -2 and -1 for common channel and veto
    ASSERT(channelNumber >= -2,-1);
    ASSERT(channelNumber != 0, -1); //Channel 0 does not exist
    ASSERT(channelNumber <= att_getInstrumentChannelNumber(timeTaggerID),-1);
    
    //The voltages that it accepts as trigger are limited
    //According the documentation, the range is -1.5V to 1.5V
    WARN(thresholdVolts > 1.5,thresholdVolts = 1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",thresholdVolts,channelNumber,1.5);
    WARN(thresholdVolts < -1.5,thresholdVolts = -1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",thresholdVolts,channelNumber,-1.5);
    
    //The documentation from Agilent's header files, copy-pasted for convenience. Legality: questionable.
    //! Configures the channel source control parameters of the Time Counter.
    /*!
     *  'channel'  = 1...nbrChannels (as returned by 'Acqrs_getNbrChannels' )
     *             = -1 for the Common Channel
     *             = -2 for the Veto Input
     *
     *  'mode'     = Bit field
     *       bit0  = 0: positive slope / 1: negative slope
     *       bit31 = 0: active channel / 1: inactive channel
     *
     *  'level'    = Threshold value in Volts
     *
     *  'reserved' = unused, must be 0
     *
     */
    //Set up the channel. The positive/negative slope is a nice little feature, but completely
    //  irrelevant to the task at hand. The threshold has range -1.5 to  1.5 volts.
    status = AcqrsT3_configChannel(timeTaggerID,(ViInt32)channelNumber,edge,(ViReal64)thresholdVolts,0);
    
    STAT_CHKERR(status,-1,"Channel %i setup failed (%X).",channelNumber,status);
    
    return 0;
}

//Enables all of the time tagger's standard channels (ie, not common channel or veto), and sets
//all of their thresholds to the thresholdVolts.
int att_setAllChannels(ViSession timeTaggerID, float thresholdVolts, int edge) {
    int i, channelNumber;
    int returnValue = 0;
    
    ASSERT(timeTaggerID != 0, -1);
    
    channelNumber = att_getInstrumentChannelNumber(timeTaggerID);
    ASSERT(channelNumber > 0, -1);
    
    for (i=1; i <= channelNumber; i++) {
        returnValue += att_setChannel(timeTaggerID,i,thresholdVolts, edge);
    }
    
    return returnValue;
}

//Disables the given channel number
int att_disableChannel(ViSession timeTaggerID, int channelNumber) {
    ViStatus status;
    
    ASSERT(channelNumber >= -2,-1);
    ASSERT(channelNumber != 0, -1); //Channel 0 does not exist
    ASSERT(channelNumber <= att_getInstrumentChannelNumber(timeTaggerID),-1);
    
    //Disable the channel - a 1 on the 31st bit.
    status = AcqrsT3_configChannel(timeTaggerID,(ViInt32)channelNumber,0x80000000, 0.0, 0);
    
    STAT_CHKERR(status,-1,"Disabling channel %i failed (%X).",channelNumber,status);
    
    return 0;
}

//Initializes the nth time tagger. Note that this initialization will only work for the TC890! (U1051A)
//  The commonNumber is 0 if it is to buffer switch on memory fill, and non-zero if it is to switch
//  on a number of common counts. The timeout is a timeout in seconds. If 0, then there is no timeout
ViSession att_initializeTimeTagger(int timeTaggerIndex,int commonNumber, int useref, double timeout, int memsize) {
    ViSession timeTaggerID;
    
    int deviceIndex = att_getTimeTaggerInstrumentIndex(timeTaggerIndex);
    
    ASSERT(deviceIndex >= 0,(ViSession)0);
    ASSERT(commonNumber >=0,(ViSession)0);
    
    //Initialize the time Tagger
    timeTaggerID = att_initializeInstrument(deviceIndex);
    
    ASSERT(timeTaggerID != 0,(ViSession)0);
    
    //The following code sets up the time tagger to be ready to start taking data
    //I attempted to use the correct options, but the T3 interface's documentation was
    //lacking, and in many cases the functions had different meanings than the well-documented
    //D1 functions, so I resorted to the basic definitions in the associated header files, as well
    //as the given sample program.
    //I have copy-pasted this documentation before each function call for simpler modification.
    //The /**/ comments are Agilent's documentation, and not mine!
    //---------------------------------------------------------------------------------------
    
    //Allows simplified error correction - the input is a call to an Acqrs function which returns a status.
    //  if an error occurs, prints an error message, closes the time tagger, and returns 0
    #define CHKERR(a) {\
        ViStatus s = a; \
        if (s != VI_SUCCESS) { \
            if (s == ACQIRIS_WARN_SETUP_ADAPTED) { \
                fprintf(stderr,"%i:%s:WARNING: '%s' Setup values adapted.\n", __LINE__, __FUNCTION__, #a); \
            } else { \
                fprintf(stderr,"%i:%s: '%s' failed (%X).\n", __LINE__, __FUNCTION__, #a,s); \
                att_closeInstrument(timeTaggerID); \
                return (ViSession)0; \
            } \
        } \
    }
    
    
    //! Configures the operational mode of the Time Counter.
    /*!
     *  'mode' = Operation mode.
     *            1: Standard acquisition
     *            2: Time Of Flight acquisition
     *
     *  'modifier'
     *      ['mode' = {1}]
     *          bit0 =    0: single hit / 1: multiple hits
     *
     *  'flags'
     *      ['mode' = {1,2}]
     *          bit0 =    0: use internal clock reference / 1: use external clock reference
     *
     */
    //I guess multiple hits is allowing multiple pulses on each channel?
    //Internal clock reference is probably what we want. Again, not sure.
    //For mode, 2 and 0 are possibilities, but 2 is used in the sample - I think it is what we want.
    //The documentation given is extremely sparse - there are no detailed explanations for 
    //  half of the settings.
    CHKERR( AcqrsT3_configMode(timeTaggerID,2,1,(useref?1:0)) );
    
    //! Configures the acquisition conditions of the Time Counter.
    /*!
     *  'timeout'  = Value of timeout in seconds. The value 0.0 means no timeout,
     *                it is not acceptable in some modes.
     *
     *  'flags'    = unused, must be 0
     *
     *  'reserved' = unused, must be 0
     *
     */
    //The D1 documentation has further clues:
    /*  Also not shown, is a call to the function  AcqrsD1_configAvgConfig  to set a timeout value for the 
        automatic completion of a segment in case the real trigger never arrives.
    */
    //I am using the value given in the sample program, but am not sure as to whether the 8 seconds has any
    //  effect here - 0 seconds seems like a good value EDIT: This is not longer relevant
    CHKERR( AcqrsT3_configAcqConditions(timeTaggerID,timeout,0,0) );
    
    
    //! Configures the memory switch conditions.
    /*!
     *  'switchEnable' = Bit field that identifies the event that switches the bank
     *                    1: switch on auxiliary I/O control.
     *                    2: switch on count of common events.
     *                    4: switch on memory size reached.
     *
     *  'countEvent'   = Number of events on common channel before switching.
     *
     *  'sizeMemory'   = Size of memory to fill before switching.
     *
     *  'reserved'     = unused, must be 0
     *
     */
    //Enable switch between banks upon filling the entire bank size, which according
    //  to the sample, is 8MB. Thus 8 megabytes it is.
    //Switch was giving values adapted when used with 8MB and 0 count events, so 
    //  it turned out that minimum count is 1, and the max memory is 8MB-4bytes.
    //  This was set to be TC890_MEMSIZE
    //The datapoints that can be held is 2 million in one readout, meaning that 
    //In order to allow semi-live streaming of data, switch on common channel is also
    //  used - and that allows data to stream, even if it is coming in at a very low rate.
    
    CHKERR( AcqrsT3_configMemorySwitch(timeTaggerID,(commonNumber?2:4),(commonNumber?commonNumber:1),memsize,0) );
    //CHKERR( AcqrsT3_configMemorySwitch(timeTaggerID,(commonNumber?2:4),(commonNumber?commonNumber:1),1*1024*1024,0) );
    /*This was used to debug to find the maximum bank size and minimum count event
    {
        ViInt32 swe,cnt,mem,res;
        AcqrsT3_getMemorySwitch(timeTaggerID,&swe,&cnt,&mem,&res);
        printf("SwitchValues: %u %u %u %u\n",swe,cnt,mem,res);
    }*/
    
    //These commands enable the common channel and the trigger. The channels are set as:
    //-1 is common channel
    //-2 is veto
    //AcqrsT3_configChannel(timeTaggerID, -1, 1, 0.7, 0);
    //AcqrsT3_configChannel(timeTaggerID, -2, (1<<31+1), 1.0, 0);

    
    //! Configures Control-IO connectors of the Time Counter.
    /*!
     *  'connector'   =  1: Front Panel I/O Aux 1
     *                =  2: Front Panel I/O Aux 2
     *                = 13: Front Panel Veto Input
     *
     *  'signal'      = value depends on 'connector', refer to manual for definitions.
     *
     *  'qualifier1'  = Bit field
     *           bit0 =  1: use 50 Ohm termination / 0: don't
     *
     *  'qualifier2'  = currently unused (set to zero!)
     *
     */
    AcqrsT3_configControlIO(timeTaggerID, 13, 1, 1, 0.0);
    
    //Now Calibrate the time tagger!
    CHKERR( Acqrs_calibrate(timeTaggerID) );
    
    //The CHKERR macro is only relevant in this function, so undefine it not to have problems later
    #undef CHKERR
    //---------------------------------------------------------------------------------------
    //Finally, the setup is finished
    
    return timeTaggerID;
}

//Given the timeTaggerID and a pointer to a readout function (a FAST readout function, mind you!)
//  this runs acquisition until either an error occurs or the readout function returns 1.
//There is also the option to send a pointer to a boolean value - while this value is not 0, data
//  acquisition will continue, when this is set to false, data acquisition will end once the acquire call ends.
//Note that to future-proof this, the readout function must be threadsafe! If it is not working fast enough,
//  the readout function will be moved to a different thread than the actual acquisition. 
//The readout function has one argument, a pointer to the data descriptor
int att_runAcquisition(ViSession timeTaggerID, ViInt32* buffer,uint64_t bufsize,uint64_t volatile *bufferloc,int volatile* keepRunning, int (*analysisfunction)(ViInt32*,int,void*),void* passvalue) {
    AqT3ReadParameters taggerReadOptions;
    AqT3DataDescriptor taggerReadoutInfo;
    //ViInt32 * dataBuffer;   //Each  time tag is 32 bits.
    ViStatus status;        //Note that this is used in CHKERR macro
    //int readoutReturn = 100;//A variable to hold the readout function's return value
    
    ASSERT(timeTaggerID != 0, -1);
	ASSERT(buffer, -1);
	ASSERT(bufsize > 2,-1);
	ASSERT(bufferloc,-1);

    //ASSERT(readoutFunction != NULL, -1);
    
    //IsRunning is allowed to be NULL, in which case the function will run until the readout function
    //  calls an end. In order to get this functionality, the pointer must be valid when it
    //  is called, and it must be non-0 at all times. Since it is readonly, this is easily achieved by
    //  setting it to point to timeTaggerID - a value that is guaranteed to never be 0
    if (keepRunning==NULL) {
        keepRunning = (int*)&timeTaggerID;
    }
    
    //A Macro that defines the steps needed to be done to clean up
	//free(dataBuffer);
    #define CLEANUP  status = AcqrsT3_stopAcquisition(timeTaggerID); STAT_CHKERR(status,-3,"Stopping Acquisition failed (%X)",status);
    //Allows simplified error correction - the input is a call to an Acqrs function which returns a status.
    //  if an error occurs, prints an error message, cleans up, and returns.
    #define CHKERR(a) status = a; \
        if (status != VI_SUCCESS) { \
            fprintf(stderr,"%i:%s: '%s' failed (%X).\n", __LINE__, __FUNCTION__, #a,status); \
            CLEANUP; \
            return -2; \
        }
    
    //Prepare the buffer to which data will be written. According to the documentation, for DMA to work,
    //the buffer must be aligned to 32 bits, meaning 4 bytes. This means (dataBuffer%4==0) for DMA.
    //From what I understand, the readout function automatically aligns the address, so in order to
    //guarantee that we have enough memory to store the entire readout, even if malloc returns an address
    //not divisible by 4, we pad it by 4 extra bytes.
    //dataBuffer = (ViInt32*)malloc(TC890_MEMSIZE+4);
    //ASSERT( dataBuffer != NULL , -1);
    
    //Set readout options
    memset(&taggerReadOptions, 0 , sizeof(taggerReadOptions));  //Clear the read options struct
    //taggerReadOptions.dataArray         = dataBuffer;           //Pointer to the buffer where data will be written
    taggerReadOptions.dataSizeInBytes   = TC890_MEMSIZE+4;      //The size of the buffer
    taggerReadOptions.dataType          = ReadRawData;          //Read raw data (direct DMA?)
    taggerReadOptions.readMode          = AqT3ReadContinuous;   //Keep acquiring data during read
    taggerReadOptions.nbrSamples		= 2097152;				//The maximum number of samples

    //Start acquisition
    CHKERR( AcqrsT3_acquire(timeTaggerID) );

    //The time tagger waits for a trigger on the common channel to start acquisition, so send the trigger
    //using software, since the common channel is disabled by default.
    //Documentation copy-pasted from Agilent's headers:
    //! Generate a COMMON hit by software. Currently only supported on TC890.
    /*!
     *    The function returns immediately after initiating a trigger.
     *    All parameters except 'instrumentID' are currently unused and must be set to 0.
    */
    CHKERR( AcqrsT3_forceTrig(timeTaggerID, 0,0,0) );
    
    //While it is to keep running, take data!
    while (*keepRunning) {
        //memset(&taggerReadoutInfo,0,sizeof(taggerReadoutInfo));
        
        //Wait until a memory bank fills and the time tagger is acquiring on the second bank, so that
        //  we can read out the acquired data.
        //Documentation copy-pasted from Agilent's headers:
        //! Waits until the end of the acquisition.
        /*!
         *  This function returns after the acquisition has terminated or after timeout,
         *  whichever comes first.
         *
         *  This function puts the calling thread into 'idle' until it returns, permitting optimal
         *  use of the CPU by other threads.
         *
         *  'timeout' = Timeout duration in milliseconds. For protection, 'timeout' is
         *              internally clipped to a range of [0, 10000] milliseconds.
         *
         *  Returns the following status values:
         *    ACQIRIS_ERROR_ACQ_TIMEOUT:         if the timeout duration has expired.
         *    ACQIRIS_ERROR_INSTRUMENT_STOPPED:  if the acquisition was not started beforehand
         *    VI_SUCCESS:                        otherwise, meaning the acquisition has ended.
         *
         */
        status = AcqrsT3_waitForEndOfAcquisition(timeTaggerID, 8000);
        
        //If there is a timeout, a read will fail and return garbage. We therefore make sure
        //  that a timeout is seen as such
        if (status != ACQIRIS_ERROR_ACQ_TIMEOUT) {
            
            WARN(status == ACQIRIS_ERROR_INSTRUMENT_STOPPED, CLEANUP; return 1;, \
                "Tagger has stopped acquisition. Exiting.");
            
			CHKERR( status );

			//Set the current DMA location in the array
			taggerReadOptions.dataArray = &(buffer[(*bufferloc % bufsize)*TC890_DMASIZE]);

			//memset(&taggerReadoutInfo, 0, sizeof(AqT3DataDescriptor));
            //Next we read out the data from the memory bank into our buffer array
            //! Returns the data acquired by the Time Counter.
            /*!
             *  'channel'  = unused, must be 0.
             *
             *  'readPar'  = pointer to a user-supplied structure that specifies what and how to read.
             *
             *  'dataDesc' = user-allocated structure for returned data and data descriptors.
             *
             *  Deprecated: Prefer the 'typed' functions 'AcqrsT3_readDataInt32' or 'AcqrsT3_readDataReal64'.
             */
            //Supposedly this is deprecated, but it is the default mode used in the sample, and it looks like
            //this is actually DMA straight into the array.
            CHKERR( AcqrsT3_readData(timeTaggerID,0,&taggerReadOptions,&taggerReadoutInfo) );
            
            //This supposedly notifies the time tagger that the memory bank was read, and can now
            //  be switched in for acquisition
            CHKERR( AcqrsT3_acquire(timeTaggerID) );
            
            if (taggerReadoutInfo.flags) {
                printf("OVERFLOW:");
                if (taggerReadoutInfo.flags & 1) {
                    printf(" Internal\n");
                } 
                if (taggerReadoutInfo.flags & 2) {
                    printf(" External\n");
                }
            }
			if (taggerReadoutInfo.dataPtr != taggerReadOptions.dataArray) {
				printf("Returned data pointer incorrect!\n");
			
			}
			//if (dataptr->microbuffer[dataptr->writeloc%(dataptr->mbufsize)+taggerReadoutInfo.nbrSamples]!=0) printf("OVERWRITE!\n");
			//if (taggerReadoutInfo.nbrSamples >= TC890_DMASIZE || taggerReadoutInfo.nbrSamples < 2097136) printf("Unexpected data sample amount! %i\n",taggerReadoutInfo.nbrSamples);
			/*for (int i=0;2097152-taggerReadoutInfo.nbrSamples-i != 0;i++) {
				dataptr->microbuffer[((dataptr->writeloc)%(dataptr->mbufsize))+2097152-1-i]=0;
			}*/

			buffer[(*bufferloc % bufsize)*TC890_DMASIZE+taggerReadoutInfo.nbrSamples] = 0;

            //Runfunction: If nonzero return value, stop acquisition
			if (analysisfunction) {if (analysisfunction(&(buffer[(*bufferloc % bufsize)*TC890_DMASIZE]),taggerReadoutInfo.nbrSamples,passvalue)) break;}

			(*bufferloc)++;

			taggerReadoutInfo.nbrSamples = 0;
        } else {
            //PRINT("TIMEOUT: No data...");
        }
    }
    
    //Do the necessary cleanup
    CLEANUP;
    
    return 0;
    
    //The macros are only relevant in this function, so undefine them not to have problems later
    #undef CHKERR
    #undef CLEANUP
}



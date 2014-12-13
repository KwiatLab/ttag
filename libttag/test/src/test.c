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

//A simple test of the functionality

#include "../../src/ttag.h"
#include <stdio.h>
#include <time.h>

#define S(x) #x
#define STRINGIFY(x) S(x)
#define TEST(a,b,t) if (a) {\
    b;\
    return __FILE__ ":" STRINGIFY(__LINE__) " ("#a "):\n\t" t;\
}

//-------------------------------------------------------------------------------------------------------------------------------------------
//Tests of functionality in buffer.c (The core of libTTag)

char* test_initial() {
    TEST(0!=tt_getBufferAmount(),,"There should be no active buffers!");
    TEST(0!=tt_getNextFree(),,"Free amount should be 0");
    TEST(0!=tt_bufferExists(0),,"Buffer should not exist!");
    TEST(-1!=tt_buffertomap(0),,"Mapping nonexistent should give error");
    return NULL;
}
char* test_basic() {
    tt_buf* buffer = tt_create(0,10);
    tt_buf* rbuffer = tt_open(0);
    TEST(buffer == NULL,,"Failed to create buffer!");
    TEST(rbuffer == NULL,tt_close(buffer);,"Failed to create read buffer!");
    TEST(10!=tt_maxdata(buffer),tt_close(buffer);tt_close(rbuffer);,"Buffer size does not match created size!");
    TEST(0!=tt_datapoints(buffer),tt_close(buffer);tt_close(rbuffer);,"Initialized buffer non-empty");
    
    tt_datanum(buffer)++;
    tt_writeindex(buffer)++;
    TEST(1!=tt_datapoints(rbuffer),tt_close(buffer);tt_close(rbuffer);,"I don't even...");
    
    tt_datanum(buffer) = 0;
    tt_writeindex(buffer) = 0;

    tt_setchannels(buffer,5);
    tt_setresolution(buffer,13.37);
    tt_setreference(buffer,3456);
    
    TEST(5!=tt_channels(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Setting channels failed");
    TEST(13.37!=tt_resolution(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Setting resolution failed");
    
    TEST(3456!=tt_reference(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Setting reference failed");
    tt_add(buffer,0,12);
    TEST(3456!=tt_reference(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Setting reference failed");
    tt_add(buffer,0,3457);
    TEST(3457!=tt_reference(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Reference update failed");
    
    TEST(0!=tt_running(buffer),tt_close(buffer);tt_close(rbuffer);,"Initial runner amount incorrect");
    tt_setrunners(buffer,5);
    TEST(5!=tt_running(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Setting runners failed");
    tt_addrunner(buffer);
    TEST(6!=tt_running(rbuffer),tt_close(buffer);tt_close(rbuffer);,"Adding runners failed");
    tt_remrunner(rbuffer);
    TEST(5!=tt_running(buffer),tt_close(buffer);tt_close(rbuffer);,"Removing runners failed");
    
    tt_close(buffer);
    tt_close(rbuffer);
    return NULL;
}

char* test_readwrite() {
    uint64_t clrtags[5] = {0,0,0,0,0};
    uint8_t clrchannels[5] = {0,0,0,0,0};
    uint64_t tags[7] = {123450,123451,123452,123453,123454,123455,123456};
    uint8_t channels[7] = {0,1,2,3,4,5,6};
    uint8_t offsetchannels[7] = {1,3,5,7,9,11,13};
    uint64_t rtags[7];
    uint8_t rchannels[7];
    
    
    tt_buf* buffer = tt_create(0,5);
    TEST(buffer == NULL,,"Failed to create buffer!");
    
    tt_channel(buffer,0) = 7;
    tt_tag(buffer,0) = 987;
    
    TEST(1!=tt_validateBuffer(buffer),tt_close(buffer);,"Empty buffer validation failed");
    
    TEST(7!=tt_channel(buffer,0),tt_close(buffer);,"WTF?");
    TEST(987!=tt_tag(buffer,0),tt_close(buffer);,"WTF?");
    
    tt_add(buffer,3,657);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");

    TEST(1!=tt_validateBuffer(buffer),tt_close(buffer);,"Buffer validation failed");
    TEST(1!=tt_datanum(buffer),tt_close(buffer);,"Datapoint number unincremented");
    TEST(1!=tt_datapoints(buffer),tt_close(buffer);,"Getting datapoints fails");
    TEST(3!=tt_channel(buffer,0),tt_close(buffer);,"Channel not written");
    TEST(657!=tt_tag(buffer,0),tt_close(buffer);,"Tag not written");
    
    //-----------------------------------------------------------------------------------
    //Tests of AddArray
    
    //Adds to the end of the buffer
    tt_addarray(buffer,channels,tags,7);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");

    TEST(1!=tt_validateBuffer(buffer),tt_close(buffer);,"Buffer validation failed");
    TEST(8!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(4!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(5!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(2!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(3!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123454!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123455!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(123452!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123453!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(4!=tt_channel(buffer,5),tt_close(buffer);,"Wraparound fails");
    TEST(123454!=tt_tag(buffer,5),tt_close(buffer);,"Wraparound fails");
    
    tt_addarray(buffer,channels,tags,2);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    TEST(10!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(4!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(5!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(0!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123454!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123455!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(123450!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123451!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    tt_addarray(buffer,channels,tags,2);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    TEST(12!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(0!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(0!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123450!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123451!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(123450!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123451!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    //The buffer should be invalid
    TEST(0!=tt_validateBuffer(buffer),tt_close(buffer);,"Buffer validation failed");
    //-----------------------------------------------------------------------------------
    //Tests of addarray with offset
    
    tt_addarray(buffer,clrchannels,clrtags,5);
    tt_datanum(buffer)=1;
    tt_writeindex(buffer)=1;

    tt_addarray_offset(buffer,offsetchannels,tags,7,-1,2);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    TEST(8!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(4!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(5!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(2!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(3!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123454!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123455!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(123452!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123453!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(4!=tt_channel(buffer,5),tt_close(buffer);,"Wraparound fails");
    TEST(123454!=tt_tag(buffer,5),tt_close(buffer);,"Wraparound fails");
    
    tt_addarray_offset(buffer,offsetchannels,tags,2,-1,2);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    TEST(10!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(4!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(5!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(0!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123454!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123455!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    
    //There is a reference flip at this point in addarray_offset
    TEST(123450+123456!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123451+123456!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    tt_addarray_offset(buffer,offsetchannels,tags,2,-1,2);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    TEST(12!=tt_datanum(buffer),tt_close(buffer);,"Datanum incorrectly incremented");
    
    TEST(0!=tt_channel(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(6!=tt_channel(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(0!=tt_channel(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(1!=tt_channel(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    TEST(123450+123451+123456!=tt_tag(buffer,0),tt_close(buffer);,"Data written incorrectly");
    TEST(123451+123451+123456!=tt_tag(buffer,1),tt_close(buffer);,"Data written incorrectly");
    TEST(123456!=tt_tag(buffer,2),tt_close(buffer);,"Data written incorrectly");
    TEST(123450+123456!=tt_tag(buffer,3),tt_close(buffer);,"Data written incorrectly");
    TEST(123451+123456!=tt_tag(buffer,4),tt_close(buffer);,"Data written incorrectly");
    
    tt_datanum(buffer) = 5;
    tt_writeindex(buffer)=5;

    tt_addarray(buffer,channels,tags,5);
    TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
    //-----------------------------------------------------------------------------------
    //Tests of readarray
    
    TEST( 5 != tt_readarray(buffer,5,rchannels,rtags,7), tt_close(buffer);,"Incorrect amount read");
    
    TEST( 123450 != rtags[0],tt_close(buffer);,"Data read incorrectly");
    TEST( 123451 != rtags[1],tt_close(buffer);,"Data read incorrectly");
    TEST( 123452 != rtags[2],tt_close(buffer);,"Data read incorrectly");
    TEST( 123453 != rtags[3],tt_close(buffer);,"Data read incorrectly");
    TEST( 123454 != rtags[4],tt_close(buffer);,"Data read incorrectly");
    
    TEST( 0 != rchannels[0],tt_close(buffer);,"Data read incorrectly");
    TEST( 1 != rchannels[1],tt_close(buffer);,"Data read incorrectly");
    TEST( 2 != rchannels[2],tt_close(buffer);,"Data read incorrectly");
    TEST( 3 != rchannels[3],tt_close(buffer);,"Data read incorrectly");
    TEST( 4 != rchannels[4],tt_close(buffer);,"Data read incorrectly");
    
    tt_datanum(buffer) = 13;
    tt_writeindex(buffer) = 13;

    TEST( 3 != tt_readarray(buffer,9,rchannels,rtags,3), tt_close(buffer);,"Incorrect amount read");
    

    TEST( 123454 != rtags[0],tt_close(buffer);,"Data read incorrectly");
    TEST( 123450 != rtags[1],tt_close(buffer);,"Data read incorrectly");
    TEST( 123451 != rtags[2],tt_close(buffer);,"Data read incorrectly");
    
    TEST( 4 != rchannels[0],tt_close(buffer);,"Data read incorrectly");
    TEST( 0 != rchannels[1],tt_close(buffer);,"Data read incorrectly");
    TEST( 1 != rchannels[2],tt_close(buffer);,"Data read incorrectly");
    
    //---------------------------------------------------------------------------------
    //Tests of readChannel
    
    {//Windows needs extra brackets here to do stuff correctly
    
        uint8_t c_channels[5]= {0,2,2,1,2};
        uint64_t c_tags[5] = {0,1,2,3,4};
        uint64_t cr_tags[5];
        
        tt_datanum(buffer) = 0;
        tt_writeindex(buffer) = 0;
        tt_addarray(buffer,c_channels,c_tags,5);
        TEST(tt_writeindex(buffer)!=tt_datanum(buffer),tt_close(buffer);,"WriteIndex and datanum out of sync");
        tt_datanum(buffer) = 12;
        tt_writeindex(buffer) = 12;

        TEST(3 != tt_readchannel(buffer,7,2,cr_tags,5),tt_close(buffer);,"Reading from channel failed");
        
        TEST( 2 != cr_tags[0],tt_close(buffer);,"Data read incorrectly");
        TEST( 4 != cr_tags[1],tt_close(buffer);,"Data read incorrectly");
        TEST( 1 != cr_tags[2],tt_close(buffer);,"Data read incorrectly");
        TEST(1 != tt_readchannel(buffer,8,2,cr_tags,1),tt_close(buffer);,"Reading from channel failed");
    }
    
    
    tt_close(buffer);
    return NULL;
}

char* test_multiple() {
    tt_buf *b0,*b1,*b3;
    
    TEST(0!=tt_getNextFree(),,"Free amount should be 0");
    b0 = tt_create(0,10);
    TEST(b0 == NULL,,"Failed to create buffer!");
    
    TEST(1!=tt_getNextFree(),,"Next Free failed");
    b1 = tt_create(1,15);
    TEST(b1 == NULL,tt_close(b0);,"Failed to create buffer!");
    
    TEST(2!=tt_getNextFree(),tt_close(b0);tt_close(b1),"Next Free failed");
    TEST(2!=tt_getBufferAmount(),tt_close(b0);tt_close(b1),"All buffers not detected");
    
    b3 = tt_create(3,10);
    TEST(b3 == NULL,tt_close(b0);tt_close(b1);,"Failed to create buffer!");
    
    TEST(2!=tt_getNextFree(),tt_close(b0);tt_close(b1);tt_close(b3),"Next Free failed");
    
    //The number of buffers should still increment
    TEST(3!=tt_getBufferAmount(),tt_close(b0);tt_close(b1);tt_close(b3),"All buffers not detected");
    
    //Test buffer2map
    TEST(0!=tt_buffertomap(0),tt_close(b0);tt_close(b1);tt_close(b3),"Invalid mapping!");
    TEST(0!=tt_buffertomap(0),tt_close(b0);tt_close(b1);tt_close(b3),"Invalid mapping!");
    TEST(1!=tt_buffertomap(1),tt_close(b0);tt_close(b1);tt_close(b3),"Invalid mapping!");
    TEST(3!=tt_buffertomap(2),tt_close(b0);tt_close(b1);tt_close(b3),"Invalid mapping!");
    TEST(-1!=tt_buffertomap(3),tt_close(b0);tt_close(b1);tt_close(b3),"Does not fail gracefully!");
    
    TEST(1!=tt_bufferExists(0),tt_close(b0);tt_close(b1);tt_close(b3),"Buffer Existence check fails");
    TEST(1!=tt_bufferExists(1),tt_close(b0);tt_close(b1);tt_close(b3),"Buffer Existence check fails");
    TEST(0!=tt_bufferExists(2),tt_close(b0);tt_close(b1);tt_close(b3),"Buffer Existence check fails");
    TEST(1!=tt_bufferExists(3),tt_close(b0);tt_close(b1);tt_close(b3),"Buffer Existence check fails");
    
    tt_close(b1);
    
    TEST(0!=tt_bufferExists(1),tt_close(b0);tt_close(b1);tt_close(b3),"Buffer Existence check fails");
    
    TEST(2!=tt_getBufferAmount(),tt_close(b0);tt_close(b1);tt_close(b3),"All buffers not detected");
    
    tt_close(b0);
    tt_close(b3);
    
    return NULL;
}

char* test_interaction() {
    tt_buf* buffer = tt_create(0,5);
    TEST(buffer == NULL,,"Failed to create buffer!");
    
    {//Windows needs extra brackets here to do stuff correctly
    
        uint8_t channels[5]= {0,2,2,1,2};
        uint64_t tags[5] = {0,1,2,3,4};
        
        tt_datanum(buffer) = 0;
        tt_writeindex(buffer) = 0;
        tt_addarray(buffer,channels,tags,5);
        tt_datanum(buffer) = 12;
        tt_writeindex(buffer) = 12;

        TEST(3 != tt_channelpoints(buffer,7,2,5),tt_close(buffer);,"Getting points from channel failed");
        
        TEST(1 != tt_channelpoints(buffer,8,2,1),tt_close(buffer);,"Getting points from channel failed");
    }
    
    {//Windows needs extra brackets here to do stuff correctly
    
        uint8_t channels[5]= {0,1,2,3,4};
        uint64_t tags[5] = {0,1,1,3,4};
        
        tt_datanum(buffer) = 0;
        tt_writeindex(buffer) = 0;
        tt_addarray(buffer,channels,tags,5);
        
        TEST(1!=tt_bins2points(buffer,4,2),tt_close(buffer);,"Getting points for time bins failed");
        TEST(3!=tt_bins2points(buffer,4,3),tt_close(buffer);,"Getting points for time bins failed");
        TEST(4!=tt_bins2points(buffer,4,4),tt_close(buffer);,"Getting points for time bins failed");
        TEST(4!=tt_bins2points(buffer,4,5),tt_close(buffer);,"Getting points for time bins failed");
        TEST(0!=tt_bins2points(buffer,60,5),tt_close(buffer);,"Out-of-range argument failed");
        
        tt_add(buffer,4,5);
        TEST(2!=tt_bins2points(buffer,5,3),tt_close(buffer);,"Getting points failed when imperfect point amount");
    }
    
    tt_close(buffer);
    
    return NULL;
}

char* test_large() {
    int i;
    int number = tt_getNextFree();
    tt_buf* buffer = tt_create(number,10000000);
    
    //Write a large amount of datapoints
    for (i=0;i<12000000;i++) {
        tt_channel(buffer,tt_datanum(buffer))=1;
        tt_tag(buffer,tt_datanum(buffer))=(uint64_t)i;
        tt_datanum(buffer)++;
        TEST(i!=(int)tt_reference(buffer),tt_close(buffer);,"Reference Failed");
        tt_setreference(buffer,(uint64_t)i+1);
        TEST(i+1!=(int)tt_reference(buffer),tt_close(buffer);,"Second reference failed")
    }
    for (i=0;i<2000000;i++) {
        TEST(i+10000000!=(int)tt_tag(buffer,i),tt_close(buffer);,"Data on large buffer failed");
    }
    for (;i<10000000;i++) {
        TEST(i !=(int)tt_tag(buffer,i),tt_close(buffer);,"Data on large buffer failed");
    }
    tt_close(buffer);
    return NULL;
}

char* test_channel() {
    uint8_t channels[10]= {0,2,2,1,3,1,1,2,5,2};
    uint64_t tags[10] = {0,1,2,3,4,5,6,7,8,9};

    tt_buf* buffer = tt_create(0,25);
    TEST(buffer == NULL,,"Failed to create buffer!");
    
    //Write the test data
    tt_addarray(buffer,channels,tags,10);  
    TEST(2 != tt_channeldist(buffer,tt_datanum(buffer)-2,2,2),tt_close(buffer);,"Failed to get ChannelDist");
    
    tt_close(buffer);
    return NULL;
}


char* test_conversions() {
    double tdelays[5] = {1.,2.,3.,4.,4.53};
    uint64_t bdelays[5] = {1,2,3,4,5};
    
    double t_delays[5] = {0.0,0.0,0.0,0.0,0.0};
    uint64_t b_delays[5]= {0,0,0,0,0};
    
    double* t__delays;
    uint64_t* b__delays;
    
    tt_buf* buffer = tt_create(tt_getNextFree(),20);
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    TEST(0!=tt_time2bin(buffer,23.0),tt_close(buffer);,"Fails when resolution not set");
    TEST(0!=tt_bin2time(buffer,2300),tt_close(buffer);,"Fails when resolution not set");
    
    tt_setchannels(buffer,5);
    
    b__delays =tt_delaytime2bin(buffer,tdelays,NULL,tt_channels(buffer));
    t__delays =tt_delaybin2time(buffer,bdelays,NULL,tt_channels(buffer));
    TEST(b__delays != NULL,tt_free(b__delays);tt_close(buffer);,"Fails when resolution not set");
    TEST(t__delays != NULL,tt_free(t__delays);tt_close(buffer);,"Fails when resolution not set");
    
    b__delays =tt_delaytime2bin(buffer,tdelays,b_delays,tt_channels(buffer));
    t__delays =tt_delaybin2time(buffer,bdelays,t_delays,tt_channels(buffer));
    TEST(b__delays != NULL,tt_free(b__delays);tt_close(buffer);,"Fails when resolution not set");
    TEST(t__delays != NULL,tt_free(t__delays);tt_close(buffer);,"Fails when resolution not set");
    
    TEST(b_delays[0] != 0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(b_delays[1] != 0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(b_delays[2] != 0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(b_delays[3] != 0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(b_delays[4] != 0,tt_close(buffer);,"Delays array written when resolution not set");
    
    TEST(t_delays[0] != 0.0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(t_delays[1] != 0.0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(t_delays[2] != 0.0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(t_delays[3] != 0.0,tt_close(buffer);,"Delays array written when resolution not set");
    TEST(t_delays[4] != 0.0,tt_close(buffer);,"Delays array written when resolution not set");
    
    tt_setresolution(buffer,0.5);
    
    TEST(46!=tt_time2bin(buffer,23.0),tt_close(buffer);,"Fails conversion");
    TEST(6.0!=tt_bin2time(buffer,12),tt_close(buffer);,"Fails conversion");
    
    b__delays =tt_delaytime2bin(buffer,tdelays,NULL,tt_channels(buffer));
    TEST(b__delays == NULL,tt_close(buffer);,"Delays array not created");
    TEST(b__delays[0] != 7,tt_free(b__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(b__delays[1] != 5,tt_free(b__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(b__delays[2] != 3,tt_free(b__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(b__delays[3] != 1,tt_free(b__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(b__delays[4] != 0,tt_free(b__delays);tt_close(buffer);,"Incorrect conversion");
    
    tt_free(b__delays);
    
    t__delays =tt_delaybin2time(buffer,bdelays,NULL,tt_channels(buffer));
    TEST(t__delays == NULL,tt_free(t__delays);tt_close(buffer);,"Delays array not created");
    TEST(t__delays[0] != -0.5,tt_free(t__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(t__delays[1] != -1.,tt_free(t__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(t__delays[2] != -1.5,tt_free(t__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(t__delays[3] != -2.0,tt_free(t__delays);tt_close(buffer);,"Incorrect conversion");
    TEST(t__delays[4] != -2.5,tt_free(t__delays);tt_close(buffer);,"Incorrect conversion");
    tt_free(t__delays);
    
    //same test, but with preallocated array
    b__delays =tt_delaytime2bin(buffer,tdelays,b_delays,tt_channels(buffer));
    TEST(b__delays != b_delays,tt_free(b__delays);tt_close(buffer);,"Delays array not created");
    TEST(b_delays[0] != 7,tt_close(buffer);,"Incorrect conversion");
    TEST(b_delays[1] != 5,tt_close(buffer);,"Incorrect conversion");
    TEST(b_delays[2] != 3,tt_close(buffer);,"Incorrect conversion");
    TEST(b_delays[3] != 1,tt_close(buffer);,"Incorrect conversion");
    TEST(b_delays[4] != 0,tt_close(buffer);,"Incorrect conversion");
    
    
    t__delays =tt_delaybin2time(buffer,bdelays,t_delays,tt_channels(buffer));
    TEST(t__delays != t_delays,tt_free(t__delays);tt_close(buffer);,"Delays array not created");
    TEST(t_delays[0] != -0.5,tt_close(buffer);,"Incorrect conversion");
    TEST(t_delays[1] != -1.,tt_close(buffer);,"Incorrect conversion");
    TEST(t_delays[2] != -1.5,tt_close(buffer);,"Incorrect conversion");
    TEST(t_delays[3] != -2.0,tt_close(buffer);,"Incorrect conversion");
    TEST(t_delays[4] != -2.5,tt_close(buffer);,"Incorrect conversion");

    //Next, we subtract reference time from the given number
    TEST(5000!=tt_subtractreference(buffer,5000),tt_close(buffer);,"Empty reference incorrectly subtracted");
    
    //Add a datapoint
    tt_add(buffer,1,3000);
    TEST(8000!=tt_subtractreference(buffer,8000),tt_close(buffer);,"Reference incorrectly subtracted");
    tt_setreference(buffer,9000);
    TEST(0!=tt_subtractreference(buffer,5000),tt_close(buffer);,"Reference incorrectly subtracted");
    TEST(3000!=tt_subtractreference(buffer,9000),tt_close(buffer);,"Reference incorrectly subtracted")
    
    tt_close(buffer);
    
    return NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------------------
//Tests of functionality in stats.c (Datapoint processing functions)

char* test_singles() {
    int i;
    uint64_t channels[5] = {0};
    uint64_t tags[10] = {100,200,300,300,400,500,600,800,900,920};
    uint8_t chann[10] = {1,3,2,4,4,3,4,3,2,4};
    tt_buf* buffer = tt_create(tt_getNextFree(),20);
    
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    //Right now there is nothing in the buffer, channels are 256 and reference is undefined. tt_singles should return 0 and an
    //  assertion failure, and tt_rawsingles should also return 0
    TEST(0!=tt_singles(buffer,5.0,NULL),tt_close(buffer);,"Singles fails on empty arrays");
    TEST(0!=tt_rawsingles(buffer,50000,NULL,0),tt_close(buffer);,"RawSingles fails on empty buffer");
    
    tt_setresolution(buffer,1e-5);
    tt_setchannels(buffer,5);
    
    TEST(0!=tt_singles(buffer,5.0,NULL),tt_close(buffer);,"Singles fails on empty arrays");
    
    //Now we do the same thing with channel array
    TEST(0!= tt_singles(buffer,5.0,channels),tt_close(buffer);,"Empty channel array fails");
    TEST(0!=channels[0],tt_close(buffer);,"Channel 1 fail");
    TEST(0!=channels[1],tt_close(buffer);,"Channel 2 fail");
    TEST(0!=channels[2],tt_close(buffer);,"Channel 3 fail");
    TEST(0!=channels[3],tt_close(buffer);,"Channel 4 fail");
    TEST(0!=channels[4],tt_close(buffer);,"Channel 5 fail");
    
    //Next, we actually test the raw singles given an array of data
    tt_addarray(buffer,chann,tags,10);
    
    TEST(920!=tt_reference(buffer),tt_close(buffer);,"Reference incorrectly set");
    
    //Test singles without channel array
    TEST(8!= tt_rawsingles(buffer,620,NULL,9),tt_close(buffer);,"Singles count incorrect");
    
    //Test WITH channel array
    TEST(8!= tt_rawsingles(buffer,620,channels,9),tt_close(buffer);,"Singles count incorrect");
    TEST(0!=channels[0],tt_close(buffer);,"Singles count channel 1 incorrect");
    TEST(0!=channels[1],tt_close(buffer);,"Singles count channel 2 incorrect");
    TEST(2!=channels[2],tt_close(buffer);,"Singles count channel 3 incorrect");
    TEST(2!=channels[3],tt_close(buffer);,"Singles count channel 4 incorrect");
    TEST(4!=channels[4],tt_close(buffer);,"Singles count channel 5 incorrect");
    
    //Clear channels
    for (i=0;i<5;i++) channels[i]=0;
    
    TEST(8!=tt_singles(buffer,1e-5*620,NULL),tt_close(buffer);,"Singles count incorrect");
    TEST(8!=tt_singles(buffer,1e-5*620,channels),tt_close(buffer);,"Singles count incorrect");
    TEST(0!=channels[0],tt_close(buffer);,"Singles count channel 1 incorrect");
    TEST(0!=channels[1],tt_close(buffer);,"Singles count channel 2 incorrect");
    TEST(2!=channels[2],tt_close(buffer);,"Singles count channel 3 incorrect");
    TEST(2!=channels[3],tt_close(buffer);,"Singles count channel 4 incorrect");
    TEST(4!=channels[4],tt_close(buffer);,"Singles count channel 5 incorrect");
    
    for (i=0;i<5;i++) channels[i]=0;
    
    tt_setreference(buffer,920+380);
    
    TEST(8!=tt_singles(buffer,1e-5*(620+380),NULL),tt_close(buffer);,"Singles count incorrect");
    TEST(8!=tt_singles(buffer,1e-5*(620+380),channels),tt_close(buffer);,"Singles count incorrect");
    TEST(0!=channels[0],tt_close(buffer);,"Singles count channel 1 incorrect");
    TEST(0!=channels[1],tt_close(buffer);,"Singles count channel 2 incorrect");
    TEST(2!=channels[2],tt_close(buffer);,"Singles count channel 3 incorrect");
    TEST(2!=channels[3],tt_close(buffer);,"Singles count channel 4 incorrect");
    TEST(4!=channels[4],tt_close(buffer);,"Singles count channel 5 incorrect");
    
    for (i=0;i<5;i++) channels[i]=0;
    
    //Going outside the bounds of the buffer should only return the datapoints within it
    TEST(10!=tt_singles(buffer,50.0,NULL),tt_close(buffer);,"Singles count incorrect");
    TEST(10!=tt_singles(buffer,50.0,channels),tt_close(buffer);,"Singles count incorrect");
    TEST(0!=channels[0],tt_close(buffer);,"Singles count channel 1 incorrect");
    TEST(1!=channels[1],tt_close(buffer);,"Singles count channel 2 incorrect");
    TEST(2!=channels[2],tt_close(buffer);,"Singles count channel 3 incorrect");
    TEST(3!=channels[3],tt_close(buffer);,"Singles count channel 4 incorrect");
    TEST(4!=channels[4],tt_close(buffer);,"Singles count channel 5 incorrect");
    
    tt_close(buffer);
    
    
    //Clear channels
    for (i=0;i<5;i++) channels[i]=0;
    
    //Now we open a small buffer to check overflow code
    buffer = tt_create(tt_getNextFree(),7);
    
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    tt_setresolution(buffer,1e-5);
    tt_setchannels(buffer,5);
    
    //Next, we actually test the raw singles given an array of data
    tt_addarray(buffer,chann,tags,10);
    
    TEST(10!=tt_datanum(buffer),tt_close(buffer);,"Datapoint number in buffer incorrect");
    
    //Test WITH channel array
    TEST(7!= tt_rawsingles(buffer,620,channels,9),tt_close(buffer);,"Singles count incorrect");
    TEST(0!=channels[0],tt_close(buffer);,"Singles count channel 1 incorrect");
    TEST(0!=channels[1],tt_close(buffer);,"Singles count channel 2 incorrect");
    TEST(1!=channels[2],tt_close(buffer);,"Singles count channel 3 incorrect");
    TEST(2!=channels[3],tt_close(buffer);,"Singles count channel 4 incorrect");
    TEST(4!=channels[4],tt_close(buffer);,"Singles count channel 5 incorrect");
    
    tt_close(buffer);
    
    return NULL;
}


char* test_coincidences() {
    
    uint64_t tags[12] = {100,150,300,351,600,649,800,852,950,1601,1800,1900};
    uint8_t chann[12] = {0,1,0,1,0,1,0,1,1,2,2,2};
    uint64_t coincidenceMatrix[9] = {0};
    uint64_t coincidenceMatrix2[9] = { 0 };
    
#define testNum 4
    uint64_t testMat[testNum][9] = {{ 4,1,1,1,5,1,1,1,3},{4,3,2,3,5,1,2,1,3},{4,4,2,4,5,3,2,3,3},{2,1,2,1,2,2,2,2,3}};
    double testTime[testNum] = {1.0,1.0,1.0,300*1e-5};
    double testRadius[testNum] = {0.0,1e-5,2e-5,2e-5};
    
    int i,j;
    
    double delays[3]= {0.,50e-5,1000*1e-5};
    
    tt_buf* buffer = tt_create(tt_getNextFree(),20);
    
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    TEST(0!=tt_coincidences(buffer,1.0,1e-3, NULL, NULL),tt_close(buffer);,"Fails when resolution unset");
    TEST(0 != tt_coincidences_nd(buffer, 1.0, 1e-3, NULL), tt_close(buffer); , "Fails when resolution unset");
    
    tt_setresolution(buffer,1e-5);
    tt_setchannels(buffer,3);
    
    
    
    tt_addarray(buffer,chann,tags,12);
    
    for (i=0;i<testNum;i++) {
        TEST(coincidenceMatrix!=tt_coincidences(buffer,testTime[i],testRadius[i],coincidenceMatrix,delays),tt_close(buffer);,"Matrix not returned");
        for (j=0;j<9;j++) {
            printf("COINCIDENCE: %i %i | >%i< == %i?\n",i,j,(int)coincidenceMatrix[j], (int)testMat[i][j]);
        }
        for (j=0;j<9;j++) {
            TEST(coincidenceMatrix[j]!=testMat[i][j],tt_close(buffer);,"Coincidence counts incorrect");
        }
        
        //Clear the matrix
        for (j=0;j<9;j++) coincidenceMatrix[j]=0;
    }

    //Make sure that the results of nodelay are the same as with delay
    for (i = 0; i<testNum; i++) {
        TEST(coincidenceMatrix != tt_coincidences(buffer, testTime[i], testRadius[i], coincidenceMatrix, NULL), tt_close(buffer); , "Matrix not returned");
        TEST(coincidenceMatrix2 != tt_coincidences_nd(buffer, testTime[i], testRadius[i], coincidenceMatrix2), tt_close(buffer);, "Matrix not returned");

        for (j = 0; j<9; j++) {
            TEST(coincidenceMatrix[j] != coincidenceMatrix2[j], tt_close(buffer); , "Coincidence counts incorrect btwn dn");
        }

        //Clear the matrix
        for (j = 0; j < 9; j++) { coincidenceMatrix[j] = 0; coincidenceMatrix2[j] = 0; }
    }
    
    tt_close(buffer);
    return NULL;
}

char* test_multicoincidences() {
    uint64_t tags[15] = {300,500,501,600,700,800,800,900,900,902,1000,1100,1200,1200,1400};
    uint8_t chann[15] = {1,1,2,3,0,3,0,1,2,4,0,2,0,3,0};
    double delays[5]= {600e-5,100e-5,300e-5,400e-5,800e-5};
    uint64_t coincidences=0;
#undef testNum
#define testNum 5
    uint64_t testCoincidences[testNum]= {3,3,2,2,0};
    double testTime[testNum] = {1.0,1.0,1.0,400*1e-5,1.0};
    double testDiameter[testNum] = {1e-5,0.,0.,0.,1e-5};
    uint8_t testChannels[2+3+2+2+2] = {0,2,0,1,3,0,2,0,1,0,4};
    int testChannelnum[testNum] = {2,3,2,2,2};
    double testdelays[5] = {0};
    
    
    int i,j,k;
    
    tt_buf* buffer = tt_create(tt_getNextFree(),20);
    
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    TEST(0!=tt_multicoincidences(buffer,testTime[0],testDiameter[0],testChannels,testChannelnum[0], NULL),tt_close(buffer);,"Fails when resolution unset");
    
    tt_setresolution(buffer,1e-5);
    tt_setchannels(buffer,5);
    
    
    tt_addarray(buffer,chann,tags,15);
    
    j=0;
    for (i=0;i<testNum;i++) {
        //Prepare the delays array
        for (k=0;k<testChannelnum[i];k++) {
            testdelays[k]=delays[testChannels[j+k]];
        }
        coincidences = tt_multicoincidences(buffer,testTime[i],testDiameter[i],&(testChannels[j]),testChannelnum[i],testdelays);
        printf("MULTICOINCIDENCE: %i %i==%i\n",i,(int)coincidences,(int)testCoincidences[i]);
        TEST(testCoincidences[i]!=coincidences,tt_close(buffer);,"Coincidence counts incorrect");
        j+=testChannelnum[i];
    }
    
    
    tt_close(buffer);
    return NULL;
}

char* test_correlation() {
    uint64_t tags[12] = {90,100,110,220,230,240,290,295,300,310,800,810};
    uint8_t chann[12] = {0,1,0,0,1,0,0,0,1,0,1,0};
    uint64_t* resarr;
    int i;
    
    tt_buf* buffer = tt_create(tt_getNextFree(),20);
    
    TEST(buffer==NULL,,"Failed to create buffer!");
    
    //TEST(0!=tt_correlate(buffer,testTime[0],testDiameter[0],testChannels,testChannelnum[0], NULL),tt_close(buffer);,"Fails when resolution unset");
    
    TEST(NULL!=tt_correlate(buffer,1.0,0.3,10,0,0,0,0,NULL),tt_close(buffer);,"Fails when resolution unset");

    tt_setresolution(buffer,1e-5);
    tt_setchannels(buffer,5);
    
    resarr=tt_correlate(buffer,1.0,0.3,10,0,0,0,0,NULL);
    TEST(NULL==resarr,tt_close(buffer);,"Fails on empty buffer");
    for (i=0;i<10;i++) {
        TEST(0!=resarr[i],tt_free(resarr);tt_close(buffer);,"Empty buffer gives nonempty result");
    }
    tt_free(resarr);

    tt_addarray(buffer,chann,tags,12);
    
    resarr=tt_correlate(buffer,1.0,11e-5,21,1,0.,0,0.,NULL);
    
    TEST(NULL==resarr,tt_close(buffer);,"Fails to find correlation");
    for (i=0;i<21;i++) {
        switch (i) {
        case 0:
            TEST(3!=resarr[i],tt_free(resarr);tt_close(buffer);,"Incorrect correlation!");
            break;
        case 5:
            TEST(1!=resarr[i],tt_free(resarr);tt_close(buffer);,"Incorrect correlation!");
            break;
        case 20:
            TEST(4!=resarr[i],tt_free(resarr);tt_close(buffer);,"Incorrect correlation!");
            break;
        default:
            TEST(0!=resarr[i],tt_free(resarr);tt_close(buffer);,"Incorrect correlation!");
            break;
        }
    }
    tt_free(resarr);
    
    tt_close(buffer);
    
    //return "UNIMPLEMENTED";
    return NULL;
}

char* test_speed() {
    clock_t tic,toc;
    uint64_t i;
    tt_buf* buffer = tt_create(tt_getNextFree(),50000000);
    double hotrawtime;
    printf("Speed test\nThis tests how fast 50M tags are written.\n\tCold-writing: Writing initial data\n\tHot-writing: Overwriting old data\n\n");
    printf("Raw Buffer Write:\n");
    tic = clock();
    for (i=0;i<50000000;i++) {
        tt_writeindex(buffer)++;
        buffer->timetag[i]= 4524646452564;
        buffer->channel[i] = 234;
        tt_datanum(buffer)++;
    }
    toc = clock();
    printf("\tCold-writing 50M tags: %fs\n",(double)(toc - tic) / CLOCKS_PER_SEC);
    tic = clock();
    for (i=0;i<50000000;i++) {
        tt_writeindex(buffer)++;
        buffer->timetag[i]= 45646452564;
        buffer->channel[i] = 214;
        tt_datanum(buffer)++;
    }
    toc = clock();
    printf("\tHot-writing 50M tags: %fs\n",(double)(toc - tic) / CLOCKS_PER_SEC);
    tt_close(buffer);

    hotrawtime = (double)(toc - tic) / CLOCKS_PER_SEC;

    buffer = tt_create(tt_getNextFree(),50000000);
    printf("Tag-Macro Write (tt_tag and tt_channel):\n");
    tic = clock();
    for (i=0;i<50000000;i++) {
        tt_writeindex(buffer)++;
        tt_tag(buffer,i)= 4524646452564;
        tt_channel(buffer,i) = 234;
        tt_datanum(buffer)++;
    }
    toc = clock();
    printf("\tCold-writing 50M tags: %fs\n",(double)(toc - tic) / CLOCKS_PER_SEC);
    tic = clock();
    for (i=50000000;i<100000000;i++) {
        tt_writeindex(buffer)++;
        tt_tag(buffer,i)= 4524646452564;
        tt_channel(buffer,i) = 234;
        tt_datanum(buffer)++;
    }
    toc = clock();
    printf("\tHot-writing 50M tags: %fs\n",(double)(toc - tic) / CLOCKS_PER_SEC);

    tt_close(buffer);

    TEST(hotrawtime > 0.50,,"This computer might not be able to write tags fast enough!")
    return NULL;
}

#define TESTNUM 13
char *(*testfnc[TESTNUM])() = {test_basic,test_initial,test_readwrite,test_multiple,test_interaction,test_large,test_channel, \
        test_conversions,test_singles,test_coincidences,test_multicoincidences,test_correlation,test_speed};

int main() {
    int i;
    int success = 0;
    char* result = NULL;
#ifndef _WIN32
    tt_deleteMap(0);
    tt_deleteMap(1);
    tt_deleteMap(2);
    tt_deleteMap(3);
#endif
    printf("LibTTag v%.2f TESTS\n\nWARNING: If there are instances of buffer running, the tests will fail.\n\n",LIBTTAG);
    
    for (i=0;i<TESTNUM;i++) {
        printf("\n------------------------------------------\n\nTest %i\n\n",i);
        fflush(stdout);
        result = testfnc[i]();
        if (result) printf("FAILED\n   %s\n",result);
        else {
            success++;
            printf("\n\nTEST PASSED\n");
        }
    }
    printf("\n---------------------------------------------\n\nDon't worry about warnings/assertions in the tests - the tests intentionally do boundary cases.\n\nTests Passed: %i/%i\n",success,TESTNUM);
    
    if (success==TESTNUM) {
        printf("Yaay! Everything seems to be working!\n");
    }
    
#ifdef _WIN32
    system("pause");
#endif
    return 0;
}

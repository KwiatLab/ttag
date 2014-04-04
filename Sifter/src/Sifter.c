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

/**************************************************************************************

                                Time Tagger Buffer
                                  Implementation

**************************************************************************************/
/*

Author: Daniel Kumor, Kwiat Group.

Note:

    TODO:
        -   Make this not-so fail. This was done in a huge rush to have it done before
            the datataking deadline
        
*/

#include "ttag.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> //Cleaning up nicely upon signals

#define ASSERT(as,r) if (!(as)) { \
    fprintf(stderr,"%i:%s: Assertion '%s' failed!\n",__LINE__,__FUNCTION__, #as ); \
    return r; \
}

#define ERROR(r,...) fprintf(stderr,"%i:%s: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\n"); \
    return r;

#define CHKERR(as,r,...) if (!(as)) { \
    fprintf(stderr,"%i:%s:ERROR: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}

#define WARN(as,r,...) if (as) { \
    fprintf(stderr,"%i:%s:WARNING: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}

int sift=1;

static void exitsignal(int signal) {
    printf("Caught Exit Signal!\n");
    sift = 0;
}

int main(int argc, char** argv) {
    printf("Starting Sifter...\n");
    tt_buf* buf0 = tt_open(0);
    tt_buf* buf1 = tt_open(1);
    tt_buf* buffer = tt_create(2,500000000);
	if (!buf0 || !buf1 || !buffer) {
		tt_close(buf0);
		tt_close(buf1);
		tt_close(buffer);
		printf("COULDN'T OPEN BUFFERS\n");
		system("pause");
		return 0;
	}
    tt_setresolution(buffer,tt_resolution(buf0));
    tt_setchannels(buffer,tt_channels(buf0)+tt_channels(buf1));
    int trigger1 = 0;
    int trigger2 = 0;
    uint64_t t1loc;
    uint64_t t2loc;
	int warning = 1;
    int64_t difference=0;
    
    if (signal(SIGINT,exitsignal) == SIG_ERR) {
        ERROR(-2,"Failed to set SIGINT!");
    }
    if (signal(SIGTERM,exitsignal) == SIG_ERR) {
        ERROR(-2,"Failed to set SIGTERM!");
    }
    
    
    printf("Waiting for trigger channel\n");
    printf("Only taking into account triggers on channel 0 starting from NOW\n");
    
    uint64_t data1loc = tt_datanum(buf0);
    uint64_t data2loc = tt_datanum(buf1);
    
    //Wait until trigger found
    while ((trigger1 ==0 || trigger2==0) && sift) {
        if (tt_datanum(buf0) > data1loc) {
            if (tt_channel(buf0,data1loc) == 0 && !trigger1) {
                trigger1++;
                t1loc = tt_tag(buf0,data1loc);
                printf("Trigger 1: %lu\n",t1loc);
            }
            data1loc++;
        }
        if (tt_datanum(buf1) > data2loc) {
            if (tt_channel(buf1,data2loc) == 0 && !trigger2) {
                trigger2++;
                t2loc = tt_tag(buf1,data2loc);
                printf("Trigger 2: %lu\n",t2loc);
            }
            data2loc++;
        }
    }
    
    if (!(trigger1 == 1 && trigger2 == 1)) {
        printf("Too many triggers!\n");
        tt_close(buf0);
        tt_close(buf1);
        tt_close(buffer);
        return 1;
    }
    
    difference = (int64_t)t1loc - (int64_t)t2loc;
    printf("Found difference (T1=T2+x): %li\n",difference);
    
    //SIFT!
    printf("Taggers are now synchronized. Stuff can be plugged back in.\n");
    printf("Data taken from now on will be synchronized\nRemember to tt_start buffer 2!\n");
    
    while (sift) {
		if (tt_running(buffer)) {
			if (tt_datanum(buf0) > data1loc && tt_datanum(buf1) > data2loc) {
				do {
					tt_writeindex(buffer)++;
					if ((int64_t)tt_tag(buf0,data1loc) <= (int64_t)tt_tag(buf1,data2loc)+difference) {
						tt_channel(buffer,tt_datanum(buffer))=tt_channel(buf0,data1loc);
						tt_tag(buffer,tt_datanum(buffer)) = tt_tag(buf0,data1loc);
						data1loc++;
					} else {
						tt_channel(buffer,tt_datanum(buffer))=tt_channel(buf1,data2loc)+tt_channels(buf0);
						tt_tag(buffer,tt_datanum(buffer)) = tt_tag(buf1,data2loc)+difference;
						data2loc++;
					}
					tt_datanum(buffer)++;
					if (tt_datanum(buf0) - data1loc > 5000000*warning || tt_datanum(buf1) - data2loc > 5000000*warning) {
							printf("WARNING: Sifter is %i behind most recent data point!\n",5000000*warning);
							warning++;
					}
				} while (tt_datanum(buf0) > data1loc && tt_datanum(buf1) > data2loc && sift && tt_running(buffer));
			} 
		} else {
			data1loc = tt_datanum(buf0);
			data2loc = tt_datanum(buf1);
		}
		
		//else if (tt_datanum(buf0) > data1loc && tt_datanum(buf1) == data2loc) {
        //    //The second buffer does not have new points, but the reference time might have changed
        //} else if () {
        //    
        //}
    }
    
    tt_close(buf0);
    tt_close(buf1);
    tt_close(buffer);
    return 0;
}

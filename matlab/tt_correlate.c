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

//tt_correlate
//	input:
//		timetagger - The nubmer of time tagger to open
//		time - The time in seconds to check for singles
//      windowradius - Radius of window within which to do correlation
//      bins - The number of bins to divide the correlation results into. Result array is of this size
//      channel 1 - The first channel
//      delay 1 - The delay of the first channel. Set to 0 if you don't want any
//      channel 2 - The second channel
//      delay 2 - The delay of the second channel.

//	output:
//		correlation - Array of size bins, containing the cross-correlation between the two specified channels

#include "mex.h"
#include "../libttag/src/ttag.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	tt_buf* buffer;
	int i;
    int timetagger;
    double time;
    double windowradius;
    int bins;
    uint8_t channel1;
    uint8_t channel2;
    double delay1;
    double delay2;
    uint64_t* correlation;
    
    if (nrhs == 8 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2]) && mxIsDouble(prhs[3]) && mxIsDouble(prhs[4]) && mxIsDouble(prhs[5]) && mxIsDouble(prhs[6]) && mxIsDouble(prhs[7])) {
        timetagger = (int)mxGetScalar(prhs[0]);
        time = (double)mxGetScalar(prhs[1]);
        windowradius = (double)mxGetScalar(prhs[2]);
        bins = (int)mxGetScalar(prhs[3]);
        channel1 = (uint8_t)mxGetScalar(prhs[4]);
        delay1 = (double)mxGetScalar(prhs[5]);
        channel2 = (uint8_t)mxGetScalar(prhs[6]);
        delay2 = (double)mxGetScalar(prhs[7]);
    } else {
        mexErrMsgTxt("Inputs:\n\tTime tag buffer number\n\tAmount of time to search (seconds)\n\tRadius of window for correlation (seconds)\n\tNumber of bins into which to put correlation results\n\tThe first channel\n\tThe first channel's delay\n\tThe second channel\n\tThe second channel's delay\nOutputs:\n\tArray of size bins, containing cross-correlation between two given channels");
        return;
    }

    buffer = tt_open(timetagger);
    if (buffer) {
        correlation = tt_correlate(buffer,time,windowradius,bins,channel1,delay1,channel2,delay2,NULL);
        
        if (correlation) {
            plhs[0] = mxCreateDoubleMatrix(1,bins,mxREAL);
            for (i=0;i<bins;i++) {
                mxGetPr(plhs[0])[i] = (double)correlation[i];
            }
        } else {
            mexErrMsgTxt("Correlation Failed! There must have been an error!");
        }
        
        tt_free(correlation);
    	tt_close(buffer);
    } else {
        mexErrMsgTxt("Unable to connect to time tag buffer! Is all necessary software running?");
    }
}

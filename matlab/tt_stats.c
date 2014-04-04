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

//tt_stats:
//  input:
//      time - use datapoints from the most recent X seconds
//      radius - corresponds to about half the jitter of the entire setup (in seconds)
//      [delays] - The delays for each channel - if a scalar, finds it automatically
//      [timetagger] - the timetagger number to open
//      [tb_radius] - For more detailed control, radius can be set in timebins
//  output:
//      singles - The singles count per channel
//      coincidence - Coincidences between channels


//This is a matlab function
#include "mex.h"

//Includes the time-tagger's library
#include "../libttag/src/ttag.h"

//Standard integer sizes are used by the timetag library
#include <stdint.h>
#include <stdlib.h>


void i2d(uint64_t* in, double* out,uint64_t len) {
    uint64_t i;
    for (i=0; i < len; i++) {
        out[i] = (double)in[i];
    }
}


//This function is just a wrapper around the tt_stats function in the ttag library

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    tt_buf* buf;
    double* delays = NULL;
    
    int timetagger = 0;
    double time =0.0;
    double dbl_radius = 0.0;
    uint64_t radius=0;
    
    uint64_t *singles, *coincidences;

    if (nrhs == 5 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[3]) && mxIsDouble(prhs[4])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
        timetagger = (int)mxGetScalar(prhs[3]);
        radius = (uint64_t)mxGetScalar(prhs[4]);
    } else if (nrhs == 4 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[3])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
        timetagger = (int)mxGetScalar(prhs[3]);
    } else if (nrhs >=2 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
    } else {
        mexErrMsgTxt("Inputs:\n\ttime in seconds of data to use\n\tpulse radius in seconds (coincidence is within +- this)\n\t[optional=0] Array of delay for each channel (if 0, it is found automatically)\n\t[optional=0] timetagger to dump from.\n\t[optional]Same as radius, except it is in time bins rather than seconds\nOutputs:\n\tSingles counts for each channel\n\t[optional] Matrix of coincidences (only returned if 2 output variables given ([a,b] = tt_stats(...))");
        return;
    }

    //Get the delays array if such an array was given:
    if (nrhs >=3 && (mxGetM(prhs[2])>1 || mxGetN(prhs[2])>1)) {
        delays = mxGetPr(prhs[2]);
    }

    buf = tt_open(timetagger);
    
    if (buf) {
    
        //Allocate arrays for singles and coincidences
        singles = calloc(sizeof(uint64_t),tt_channels(buf));
        coincidences = calloc(sizeof(uint64_t),tt_channels(buf)*tt_channels(buf));
        
        if (!singles || !coincidences) {
            tt_close(buf);
            mexErrMsgTxt("Memory allocation failed");
        }
        
        tt_stats(buf,time,dbl_radius,radius,singles,coincidences,delays);
        
        //Get the output matrices
        plhs[0] = mxCreateDoubleMatrix(1,(int)tt_channels(buf),mxREAL);
        plhs[1] = mxCreateDoubleMatrix((int)tt_channels(buf),(int)tt_channels(buf),mxREAL);
        
        //Convert the integers to doubles
        i2d(singles,mxGetPr(plhs[0]),(uint64_t)tt_channels(buf));
        i2d(coincidences,mxGetPr(plhs[1]),(uint64_t)tt_channels(buf)*(uint64_t)tt_channels(buf));

        //Close the buffer
        tt_close(buf);
        
        //Frees the arrays
        free(singles);
        free(coincidences);
        
    } else {
        mexErrMsgTxt("Unable to connect to time tagger - is the timetagger software running?");
    }
}


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

//tt_delays:
//  input:
//      time - use datapoints from the most recent X seconds
//      radius - corresponds to about half the jitter of the entire setup (in seconds)
//      [timetagger] - the timetagger number to open
//      [tb_radius] - For more detailed control, radius can be set in timebins
//
//  output:
//      delays - an array of the delays of each channel, such that there is maximal coincidence

//This is a matlab function
#include "mex.h"

//Includes the time-tagger's library
#include "../libttag/src/ttag.h"

//The function is actually a wrapper around the delay function that is already in the
//  default timetagger library

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    tt_buf* buf;
    double* delays;
    
    int timetagger = 0;
    double time = 0.0;
    double dbl_radius = 0.0;
    uint64_t radius = 0;
    

    if (nrhs == 4 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2]) && mxIsDouble(prhs[3])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
        timetagger = (int)mxGetScalar(prhs[2]);
        radius = (uint64_t)mxGetScalar(prhs[3]);
    } else if (nrhs == 3 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
        timetagger = (int)mxGetScalar(prhs[2]);
    } else if (nrhs == 2 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1])) {
        time = mxGetScalar(prhs[0]);
        dbl_radius = mxGetScalar(prhs[1]);
    } else {
        mexErrMsgTxt("Inputs:\n\ttime in seconds to calculate delays from\n\tpulse radius in seconds (coincidence is within +- this)\n\t[optional=0] timetagger to dump from.\n\t[optional]Same as radius, except it is in time bins rather than seconds\nOutputs:\n\tArray of delays between channels");
    }
    
    buf = tt_open(timetagger);
    if (buf) {
        
        plhs[0]=mxCreateDoubleMatrix(1,tt_channels(buf),mxREAL);
        delays = mxGetPr(plhs[0]);
        
        tt_delays(buf,time,dbl_radius,radius,delays);
        
        tt_close(buf);
    } else {
        mexErrMsgTxt("Unable to connect to time tagger - is the timetagger software running?");
    }

}

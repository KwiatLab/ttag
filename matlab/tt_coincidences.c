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

//tt_coincidences
//  input:
//      timetagger - The nubmer of time tagger to open
//      time - The time in seconds to check for coincidences
//      radius - The time which is considered to be equivalent for coincidence
//      delays - [optional] Array of size channelNumber giving the delays between channels

//  output:
//      coincidences - Matrix of coincidences. Diagonal terms are the singles,
//          which include the delays in the calculation

#include "mex.h"
#include "matrix.h"
#include "../libttag/src/ttag.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    tt_buf* buffer;
    int timetagger;
    double runtime;
    double radius;
    double* delays = NULL;
    uint64_t* coincidences;
    int i;

    if ((nrhs == 3 || nrhs == 4) && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2])) {
        timetagger = (int)mxGetScalar(prhs[0]);
        runtime = mxGetScalar(prhs[1]);
        radius = mxGetScalar(prhs[2]);
    } else {
        mexErrMsgTxt("Inputs:\n\tTime tag buffer number\n\tAmount of time to search (seconds)\n\tRadius of coincidence\n\t[optional] Array of delays between channels\nOutputs:\n\tMatrix of coincidences between channels, with diagonal being singles");
        return;
    }

    buffer = tt_open(timetagger);
    if (buffer) {
        //Allows delays to be a subset of channels
        if (nrhs==4 && (mxGetM(prhs[3])>1 || mxGetN(prhs[3])>1)) {
            delays = (double*)calloc(tt_channels(buffer),sizeof(double));
            for (i = 0 ; i < mxGetM(prhs[3])*mxGetN(prhs[3]) && i < tt_channels(buffer); i++) {
                delays[i] = mxGetPr(prhs[3])[i];
            }
        }
        //Find the singles
        coincidences = tt_coincidences(buffer,runtime,radius,NULL,delays);

        if (coincidences) {
            plhs[0] = mxCreateDoubleMatrix(tt_channels(buffer),tt_channels(buffer),mxREAL);
            for (i=0;i<tt_channels(buffer)*tt_channels(buffer);i++) {
                mxGetPr(plhs[0])[i] = (double)coincidences[i];
            }
        } else {
            mexErrMsgTxt("Finding coincidences failed! There must have been an error!");
        }

        tt_free(coincidences);
        tt_close(buffer);
    } else {
        mexErrMsgTxt("Unable to connect to time tag buffer! Is all necessary software running?");
    }
}

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

//tt_multicoincidences
//	input:
//		timetagger - The nubmer of time tagger to open
//		time - The time in seconds to search
//      diameter - A diameter in which to search for coincidences
//      channels - An array of channels for which to find coincidences
//      delays - [optional] Array of delays for the given channels

//	output:
//		coincidences - The number of coincidences between the given channels

#include "mex.h"
#include "matrix.h"
#include "../libttag/src/ttag.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	tt_buf* buffer;
    int timetagger;
    double runtime;
    double diameter;
    uint8_t channels[256];
    double* delays = NULL;
    uint64_t result;
    int i;
	
    if (nrhs == 4 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2]) && (mxGetM(prhs[3])>1 || mxGetN(prhs[3])>1)) {
        timetagger = (int)mxGetScalar(prhs[0]);
        runtime = mxGetScalar(prhs[1]);
        diameter = mxGetScalar(prhs[2]);
        for (i=0;i<(int)mxGetM(prhs[3])*(int)mxGetN(prhs[3]);i++) {
            channels[i] = (uint8_t)(mxGetPr(prhs[3])[i]);
        }
    } else if (nrhs == 5 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1]) && mxIsDouble(prhs[2]) && (mxGetM(prhs[3])>1 || mxGetN(prhs[3])>1) && (mxGetM(prhs[4])>1 || mxGetN(prhs[4])>1)) {
        timetagger = (int)mxGetScalar(prhs[0]);
        runtime = mxGetScalar(prhs[1]);
        diameter = mxGetScalar(prhs[2]);
        for (i=0;i<(int)mxGetM(prhs[3])*(int)mxGetN(prhs[3]);i++) {
            channels[i] = (uint8_t)(mxGetPr(prhs[3])[i]);
        }
		if ((int)mxGetM(prhs[3])*(int)mxGetN(prhs[3])!=(int)mxGetM(prhs[4])*(int)mxGetN(prhs[4])) {
			mexErrMsgTxt("Number of channels does not match number of delays!");
			return;
		}
        delays = mxGetPr(prhs[4]);
    } else {
        mexErrMsgTxt("Inputs:\n\tTime tag buffer number\n\tAmount of time to search (seconds)\n\tDiameter of coincidence\n\tArray of channels for which to find coincidences\n\t[optional] Array of delays between the *given* channels\nOutputs:\n\tNumber of coincidences");
        return;
    }
	
    buffer = tt_open(timetagger);
    if (buffer) {
        result = tt_multicoincidences(buffer,runtime,diameter,channels,(int)mxGetM(prhs[3])*(int)mxGetN(prhs[3]),delays);

        plhs[0] = mxCreateDoubleMatrix(1,1,mxREAL);
        *mxGetPr(plhs[0]) = (double)result;

    	tt_close(buffer);
    } else {
        mexErrMsgTxt("Unable to connect to time tag buffer! Is all necessary software running?");
    }
}

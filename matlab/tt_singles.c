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

//tt_singles
//	input:
//		timetagger - The nubmer of time tagger to open
//		time - The time in seconds to check for singles

//	output:
//		singles - Array of per-channel counts

#include "mex.h"
#include <stdlib.h>
#include "../libttag/src/ttag.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	tt_buf* buffer;
    int timetagger;
    double runtime;
    uint64_t* singles;
    int i;

    if (nrhs == 2 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1])) {
        timetagger = (int)mxGetScalar(prhs[0]);
        runtime = mxGetScalar(prhs[1]);
    } else {
        mexErrMsgTxt("Inputs:\n\tTime tag buffer number\n\tAmount of time to search (seconds)\nOutputs:\n\tArray of per-channel pulsenumbers");
        return;
    }

    buffer = tt_open(timetagger);
    if (buffer) {
		//Allocate a singles array
		singles = (uint64_t*)calloc(sizeof(uint64_t),tt_channels(buffer));
    	//Find the singles
    	tt_singles(buffer,runtime,singles);

    	if (singles) {
    		plhs[0] = mxCreateDoubleMatrix(1,tt_channels(buffer),mxREAL);
    		for (i=0;i<tt_channels(buffer);i++) {
    			mxGetPr(plhs[0])[i] = (double)singles[i];
    		}
    	} else {
	        mexErrMsgTxt("Memory allocation failed!");
	    }

    	free(singles);
    	tt_close(buffer);
    } else {
        mexErrMsgTxt("Unable to connect to time tag buffer! Is all necessary software running?");
    }
}
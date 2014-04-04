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

//tt_start:
//  input:
//      int: number of timetagger to attempt opening
//  output:
//      int: Number of runners
//
//  If the buffer is on, start taking data. In libTTag terms, add a runner.

//This is a matlab function
#include "mex.h"

//Includes the time-tagger's library
#include "../libttag/src/ttag.h"

//Standard integer sizes 
//The default matlab compiler on windows does not have these - you will need to either download it
//  or use MSVC
#include <stdint.h>

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    //Initialize the memory map where the data points are buffered, and read the datapoints from it
    tt_buf* buf;
    int timetagger;
    
    if (nrhs == 1 && mxIsDouble(prhs[0])) {
        //If there is only one argument, and it is a double, then change the time tagger number
        //to open
        timetagger = (int)mxGetScalar(prhs[0]);
    } else {
        //If the arguments are wrong, display a help message and exit
        mexErrMsgTxt("Inputs:\n\tNumber of timetagger (starting from 0) to open.\nOutputs:\n\tNumber of runners (int)");
        return;
    }

    //Open the time tagger
    buf = tt_open(timetagger);

    if (buf) {
        
        //Add a runner to the time tagger
        tt_addrunner(buf);
        
        //If there is a left hand side argument, return the number of runners
        if (nlhs == 1) {
            plhs[0] = mxCreateDoubleMatrix(1,1,mxREAL);
            *mxGetPr(plhs[0]) = (double)tt_running(buf);
        }
        
        //Close the buffer
        tt_close(buf);
        
	} else {
        mexErrMsgTxt("Unable to connect to time tagger - is the timetagger software running?");
	}
}

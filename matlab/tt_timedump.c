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

//tt_dump:
//  input:
//      timetagger - the number of time tagger to open
//      time - the time in seconds of data to read out from the buffer (must be less than the total buffer size).

//  output:
//      channels - an array with the channel numbers for each datapoint
//      timetags - the corresponding array of time-tag timings, where channels[i] corresponds to timetags[i].

#include "mex.h"
#include "../libttag/src/ttag.h"

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    tt_buf* buffer;
    int timetagger;
    double datatime;
    uint64_t datapoints;

    
    uint64_t total;	//The total amount of data in the buffer
    uint64_t i;		//Temporary loop variable
    uint64_t j=0;
    double *channels=NULL, *timebins;
    
    if (nrhs == 2 && mxIsDouble(prhs[0]) && mxIsDouble(prhs[1])) {
        timetagger = (int)mxGetScalar(prhs[0]);
        datatime = mxGetScalar(prhs[1]);
    } else {
        mexErrMsgTxt("Inputs:\n\tTime tag buffer to dump from.\n\tTime from which to dump data.\nOutputs:\n\tArray of time stamps\n\tArray of corresponding channels");
        return;
    }
    
    if (datapoints==0) {
        mexErrMsgTxt("No datapoints were specified!");
        return;
    }
    
    buffer = tt_open(timetagger);
    if (buffer) {
        //Read the array of data
        if (isnan(tt_resolution(buffer))) {
            mexErrMsgTxt("Resolution not set - unable to convert time to data points!");
        } else {
            total = tt_datapoints(buffer);
			if (!total) { mexErrMsgTxt("There are no data points on the buffer!"); tt_close(buffer);return;}
            //Here is the main difference between time dump and dump: It first gets the number of data points from the time
            datapoints = tt_bins2points(buffer,total-1,tt_time2bin(buffer,datatime));
            
            if (datapoints > total) {
                mexPrintf("Warning: Not enough data points! Only reading saved data points!\n");
                datapoints = total;
            }
            if (datapoints > tt_maxdata(buffer)) {
                mexPrintf("Warning: There are not enough datapoints saved. Only reading to end of buffer!\n");
                datapoints = tt_maxdata(buffer);    //Only read up to the entire buffer
            }

            
            if (nlhs>=2) {
                plhs[0] = mxCreateDoubleMatrix(1,(int)datapoints,mxREAL);
                channels = mxGetPr(plhs[0]);
                plhs[1] = mxCreateDoubleMatrix(1,(int)datapoints,mxREAL);
                timebins = mxGetPr(plhs[1]);
            } else {
                plhs[0] = mxCreateDoubleMatrix(1,(int)datapoints,mxREAL);
                timebins = mxGetPr(plhs[0]);
            }
            
            //Read the array of data
            
            if (channels) {
                for (i=total-datapoints;i<total;i++) {
                    channels[j] = (double)tt_channel(buffer,i)+1.0;	//Matlab has this weird off by one thing...
                    timebins[j] = (double)tt_tag(buffer,i)*tt_resolution(buffer);
                    j++;
                }
            } else {
                for (i=total-datapoints;i<total;i++) {
                    timebins[j] = (double)tt_tag(buffer,i)*tt_resolution(buffer);
                    j++;
                }
            }
        }
        
        
        tt_close(buffer);
        
    } else {
        mexErrMsgTxt("Unable to connect to time tag buffer! Is all necessary software running?");
    }

}


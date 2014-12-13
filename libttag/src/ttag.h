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

                               Time Tagger Connection
                                    Definitions

**************************************************************************************/

//If all assertions are to be taken out, define NDEBUG:
//#define NDEBUG

//If this is to be compiled as a Windows DLL, define ISDLL
//#define ISDLL

//If it is to be a DLL, do the necessary definitions
#if defined(ISDLL)
#define TT_DEF_ extern "C" __declspec(dllexport)
#else
#define TT_DEF_
#endif

#ifndef LIBTTAG
#   define LIBTTAG 1.0f

//C99's standard integer types are extremely useful. If a compiler complains about not being able to find
//  it, just google it and download a copy of the header, they are very easy to obtain.
#include <stdint.h>
#include <math.h>   //Allows for using NaNs

#ifdef _WIN32
    #include <Windows.h>    //Windows has its own API, which defines HANDLE, so it needs to be in the header.
#endif


//Silly MSVC doesn't have NaNs!
#ifndef NAN
#include <float.h>
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif

//Note: This might fail if a compiler does a bit too much optimization!
#ifndef isnan
#define isnan(x) ((x)!=(x))
#endif


//These definitions are used 
#ifdef _WIN32
    #define TT_MAPNAMESTR "TimeTagger%i"
    #define TT_MAPMAX 16    //Minimum 13
#else
    //We want to use the globally held memory
    #define TT_MAPNAMESTR "/TimeTagger%i"
    #define TT_MAPMAX 16    //Minimum 14
    #define TT_SHM_MODE 0777
#endif


//The buffer object. 
//Note: the elements of the struct are subject to change at any point in time, so use the functions!
typedef struct { 
    uint64_t *map;     //Pointer to the beginning of shared memory
    
    //These pointers save a couple additions every time the data is accessed/written
    uint64_t *timetag;  //Pointer to location of timetag array
    uint8_t *channel;   //Pointer to location of channel array
    
#ifdef _WIN32
    HANDLE fileDescriptor;
#else
    int fileDescriptor;
    
    //These are needed to close the buffer after use
    char isCreator;         //Whether or not the object was created by this process
    char name[TT_MAPMAX];   //The name of the buffer
#endif

} tt_buf;

//-------------------------------------------------------------------------------------------------------------
//Map Detection and buffer counting

#ifndef _WIN32
/*
tt_deleteMap: A debugging function to delete a leftover linux map after a crash or something - hopefully
        this will be the least-used function in the whole library!
*/
TT_DEF_ void tt_rawdeleteMap(char* mapname);
TT_DEF_ void tt_deleteMap(int mapnumber);
#endif

/*
tt_getNextFree: Returns the number of the first available free buffer, and if all are taken returns -1.

Example:
    int mapnumber = tt_getNextFree();
    tt_create(mapnumber,50000000);
*/
TT_DEF_ int tt_getNextFree();

/*
tt_getBufferAmount: Returns the number of detected buffers - meaning that most likely that is the number
        of time taggers connected to the computer. Note that buffers created using tt_rawcreate will NOT be
        detected unless they are named using the standard buffer convention

Example:
    if (tt_getBufferAmount()==0) {
        printf("No buffers detected!");
    }
*/
TT_DEF_ int tt_getBufferAmount();

/*
tt_buffertomap: This function takes an open buffer index and converts it to buffer number. You get the
        number of open buffers using tt_getBufferAmount, and can get the mapnumber of any of them using this function.
        This means that the programmer does not have to worry about skips in the mapnumbers, and deals just in the open buffers.
        For example, if buffers 0,1,5,6 were open, with a mapnumber, one would have to know to open 0,1,5,6. This function
        allows to use 0,1,2,3 which maps to 0,1,5,6. returns -1 on failure
    buffernum: int from 0 to tt_getBufferAmount()-1, the number of open buffer for which to give a mapnumber

Example:
    int mapnum = tt_getMapnumFromBuffernum(5);
    tt_open(mapnum);
*/
TT_DEF_ int tt_buffertomap(int buffernum);

/*
tt_bufferExists: Checks if the given buffer number exists. Returns 0 if doesn't exist and x>0 if exists.
    mapnumber: An integer x, 99>=x>=0 which identifies the buffer.

Example:
    if (tt_bufferExists(0)) {
        printf("Buffer 0 already exists!");
    }
*/
TT_DEF_ int tt_bufferExists(int mapnumber);

/*
tt_mapExists: Checks whether a map with the given name already exists. Returns x>0 if exists, and 0 if it does
        not exist.
    mapname: string, the name of the buffer that one is searching for. The string length must
        be less than TT_MAPMAX.

Note: This can be used in conjunction with tt_rawcreate and tt_rawopen, but the non-raw
        functions are recommended.

WARNING: This function behaves differently on different operating systems! Namely, to get Windows-like functionality
        on UNIX, the mapname needs to be prefaced with "/", so "TimeTagger0" (Windows) -> "/TimeTagger0" (UNIX)

Example:
    if (tt_mapExists("TimeTagger0")) {
        printf("Map exists!");
    }
*/
TT_DEF_ int tt_mapExists(const char *const mapname);


//-------------------------------------------------------------------------------------------------------------
//Creating, opening and closing buffers.

/*
tt_open: Opens an existing buffer of datapoints for reading/writing. Returns NULL if the specified map 
        doesn't exist.
    mapnumber: An integer x, 99>=x>=0 which identifies the time tagger. Allows multiple time tagger buffers.
    
Example:
    tt_buf* buffer = tt_open(0);
*/
TT_DEF_ tt_buf* tt_open(const int mapnumber);

/*
tt_rawopen: Opens an existing buffer of datapoints for reading/writing. Returns NULL if the specified map 
        doesn't exist.
    mapname: A string, the name of the map. This is a global identifier, so it is important that it is unique for
        each time tagger. Furthermore, it should probably include some string identifying it as a time tagger,
        such as "timeTagger", since it names a system object. Note that the string length must be smaller than TT_MAPMAX.

Note: It is recommended to use the tt_open function for newer programs rather than this. It is here for
    compatibility with the old version.
    
WARNING: This function behaves differently on different operating systems! Namely, to get Windows-like functionality
        on UNIX, the mapname needs to be prefaced with "/", so "TimeTagger0" (Windows) -> "/TimeTagger0" (UNIX)
        
Example:
    tt_buf* buffer = tt_rawopen("TimeTagger0");
*/
TT_DEF_ tt_buf* tt_rawopen(const char *const mapname);

/*
tt_close: Closes a buffer, and if it is the last connected process, disposes of the buffer. Needs to be done
        to every buffer object before exiting.
    buffer: Pointer to the buffer object

Example:
    tt_buf* buffer = tt_open(0);
    tt_close(buffer);
*/
TT_DEF_ void tt_close(const tt_buf *const buffer);

/*
tt_create: Creates a new buffer with the given size and prepares it for usage. This function fails (returns NULL)
        if a buffer with the given number already exists, or if the specified number of datapoints will not fit
        in memory.
    mapnumber: An integer x, 99>=x>=0 which identifies the time tagger. Allows multiple time tagger buffers.
    datapoints: The number of datapoints the map is to hold. A recommended value is >= 50,000,000 in order to
        minimize the chance that the buffer will overwrite data that is being used.
        
Example:
    tt_buf* buffer = tt_create(0,50000000);
*/
TT_DEF_ tt_buf* tt_create(const int mapnumber,const uint64_t datapoints);

/*
tt_rawcreate: Creates a new buffer with the given size and prepares it for usage. This function fails (returns NULL)
        if a map with the given name already exists, or if the specified number of datapoints will not fit
        in memory.
    mapname: A string which identifies the time tagger. Be careful to make this unique!
    datapoints: The number of datapoints the map is to hold. A recommended value is >= 50,000,000 in order to
        minimize the chance that the buffer will overwrite data that is being used.
        
Note: It is recommended that tt_create be used rather than this.

WARNING: This function behaves differently on different operating systems! Namely, to get Windows-like functionality
        on UNIX, the mapname needs to be prefaced with "/", so "TimeTagger0" (Windows) -> "/TimeTagger0" (UNIX)

Example:
    tt_buf* buffer = tt_rawcreate("TimeTagger0",50000000);
*/
TT_DEF_ tt_buf* tt_rawcreate(const char *const mapname,const uint64_t datapoints);


//-------------------------------------------------------------------------------------------------------------
//Simplified Data Access
//Does not check anything, it simply returns the values
//These three definitions allow very simple data access. They are made to allow someone to write straight to
//  the shared memory. Be warned though, the convenience comes at the price of needing to know what you're doing! When writing datapoints,
//  you will need to update the datanum variable (after writing data) to a new total amount. All of these variables are memory
//  aligned, so if at all possible, make sure updating values is an atomic operation!

#define tt_channel(buffer,datapoint)    ((buffer)->channel[(datapoint)%((buffer)->map[0])])
#define tt_tag(buffer,datapoint)        ((buffer)->timetag[(datapoint)%((buffer)->map[0])])
#define tt_datanum(buffer)              ((buffer)->map[1])
#define tt_writeindex(buffer)	        ((buffer)->map[3])
/*
Example of adding a data point:

tt_writeindex(buffer)++;							//Notifies of an impending write: Invalidates the old data
tt_channel(buffer,tt_datanum(buffer))=<CHANNEL>		//Writes the channel
tt_tag(buffer,tt_datanum(buffer))=<TAG>				//Writes the time tag
tt_datanum(buffer)++;								//Notifies that the new data point is ready

NOTE: These macros are not recommended for general use. Look below for the recommended array-based tt_writearray
*/


//-------------------------------------------------------------------------------------------------------------
//Getting Buffer Information and Setting Buffer Variables

/*
tt_buffersize: Returns the given buffer's size in bytes. This should be within a couple kilobytes of
        size in memory.
    buffer: Pointer to a buffer object.

Example:
    tt_buf* buffer = tt_create(0,50000000);
    uint64_t size = tt_bufsize(buffer);
*/
TT_DEF_ uint64_t tt_buffersize(const tt_buf *const buffer);

/*
tt_maxdata: Returns the maximum number of data points that can fit in the buffer at any point in time.
    buffer: Pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    if (50000000 == tt_maxdata(buffer)) {
        //TRUE
    }
*/
TT_DEF_ uint64_t tt_maxdata(const tt_buf *const buffer);

/*
tt_datapoints: The total number of data points that have ever been handled by the buffer. Note that it is not necessarily
        the number of datapoints currently *IN* the buffer, which is limited by the buffer's size.
    buffer: Pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    if (tt_datapoints(buffer)==0) {
        //TRUE
    }
*/
TT_DEF_ uint64_t tt_datapoints(const tt_buf *const buffer);

/*
tt_datapoints: Sets the total number of data points that have ever been handled by the buffer (see tt_datapoints). This function
        is only provided for completeness, I do not see any reason to use it whatsoever, unless you are embedding libTTag in
        another language, and need a way to manually change the number of data points on the buffer after writing right to the arrays
    buffer: Pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    tt_setdatapoints(buffer,500);
    if (tt_datapoints(buffer)==500) {
        //TRUE
    }
*/
TT_DEF_ void tt_setdatapoints(const tt_buf *const buffer, uint64_t datapoints);

/*
tt_minindex: The minimum allowable index on the buffer. Since there is a rotating buffer, old points are overwritten. This returns
        the index of the oldest valid datapoint (the oldest datapoint that was not yet overwritten).
    buffer: Pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    
    tt_add(buffer,1,1);
    tt_add(buffer,5,5);
    
    if (tt_minindex(buffer)==0) {
        //TRUE
    }
*/
#define tt_minindex(buffer) (tt_writeindex(buffer) > tt_maxdata(buffer)?tt_writeindex(buffer)-tt_maxdata(buffer):0)

/*
tt_resolution: Returns the timetagger's resolution, meaning the amount of time in seconds it takes for
        one 'tick' of data. This is initialized to a NaN when no resolution is defined. It is your
        responsibility to make this a real value if any time-dependent functions are going to be used
    buffer: Pointer to a buffer object
    
Example:
    tt_buf* buffer = tt_create(0,50000000);
    if (isnan(tt_resolution(buffer))) {
        //TRUE
    }
*/
TT_DEF_ double tt_resolution(const tt_buf *const buffer);


/*
tt_setresolution: Sets the timetagger's resolution (see tt_resolution). This should not be changed during runtime,
        since the buffer can only accept data taken at one resolution at a time. If one were to change it during
        runtime, the results for all datapoints taken with the old resolution would be invalid. This value can be set
        at any time, but it must be before any functions that rely on time-based measurements are called.
    buffer: Pointer to a buffer object
    resolution: A double floating point value which gives the number of seconds per 'tick' of the timetagger

Example:
    tt_buf* buffer = tt_create(0,50000000);
    tt_setresolution(buffer,1e-5);
    if (tt_resolution(buffer)==1e-5) {
        //TRUE
    }
*/
TT_DEF_ void tt_setresolution(const tt_buf *const buffer,const double resolution);

/*
tt_channels: Returns the total number of channels the timetagger has (or the amount of channels it is currently
        taking data on). This is between 1 and 256. It is initialized to 256 by default. To speed up certain
        calculations, it is recommended that you set this to the actual number of channels - or to the number
        of channels currently being used. There are no checks for larger values in the actual data stream,
        so it is important to note that all data will actually be put on the buffer (and take up space on the buffer!)
        This value IS taken into account when running analysis functions, so the best value is as small as will fit all inputs.
    buffer: Pointer to buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    if (tt_channels(buffer)==256) {
        //TRUE
    }
*/
TT_DEF_ int tt_channels(const tt_buf *const buffer);

/*
tt_setchannels: Sets the number of channels the timetagger has (see tt_channels). Feel free to change it during runtime 
        as needed, but note that it will not change the actual data on the buffer - all data is accepted no matter what
        this value is. The reason for it is to make calculations that correlate channels faster, knowing how
        many channels to take into account.
    buffer: pointer to buffer object
    int: Number of channels

Example:
    tt_buf* buffer = tt_create(0,50000000);
    tt_setchannels(buffer,16);
    if (tt_channels(buffer)==16) {
        //TRUE
    }
*/
TT_DEF_ void tt_setchannels(const tt_buf *const buffer, const int channels);

/*
tt_reference: Returns the "current" time. Sometimes a time tagger stops taking tags - and the datapoints in the buffer are old.
        This function attempts to combat this type of problem by allowing timetagger software to set a "reference" time, which
        is a "now" timetag. This does not have to be updated when datapoints are streaming in, since reference returns the greatest
        of reference time and timetags - so when there are newer timetags than reference, the timetags are automatically set as the
        reference.
    buffer: pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    tt_add(buffer,0,65754);
    if (tt_reference(buffer)==65754) {
        //TRUE
    }
*/
TT_DEF_ uint64_t tt_reference(const tt_buf *const buffer);

/*
tt_setreference: Set the "current" time. See tt_reference for detailed explanation. This is most useful when timetagger is not getting
        datapoints, allowing the buffer to "know" that its timetags are 'old'. Remember, that if there are tags with a timestamp that
        is newer than the reference, tt_reference will return the timestamp of the newest tag.
    buffer: pointer to a buffer object

Example:
    tt_buf* buffer = tt_create(0,50000000);
    tt_setreference(buffer,0,6554);
    if (tt_reference(buffer)==6554) {
        //TRUE
    }
*/
TT_DEF_ void tt_setreference(const tt_buf *const buffer, const uint64_t ref);

/*
tt_running: Returns the number of processes that want the time tagger to be taking data (after all, if no processes
        want new data, then there is no reason to take it!) This 'pauses' data taking until tt_running is non-zero.
        Of course, this is dependent on implementation on the timetagger side, but all time taggers I programmed checked
        this every iteration. Note that the number of processes in this value is limited to max of a 16 bit unsigned integer.
    buffer: Pointer to buffer object

Example:
    tt_buf* buffer = tt_open(0);
    
    while (true) {
        while (tt_running(buffer)) {
            //Take data
        }
        sleep(0.5);
    }
*/
TT_DEF_ int tt_running(const tt_buf *const buffer);

/*
tt_setrunners: Sets the amount of runners to the given value (see tt_running). This can be changed by any process.
    buffer: Pointer to buffer object

Example:
    tt_buf* buffer = tt_open(0);
    
    tt_setrunners(buffer,10);
    if (tt_running(buffer)==10) {
        //TRUE
    }
*/
TT_DEF_ void tt_setrunners(const tt_buf *const buffer, const int runners);


/*
tt_addrunner: Increments tt_running by one (see tt_running). This should be called once per process that wants data to
        be streaming from the time tagger. If the amount of runners is already at maximum, a warning is shown
        but nothing happens. This function is threadsafe.
    buffer: Pointer to buffer object

Example:
    tt_buf* buffer = tt_open(0);
    int a = tt_running(buffer);
    tt_addrunner(buffer);
    if (tt_running(buffer)+1 == a) {
        //TRUE
    }
*/
TT_DEF_ void tt_addrunner(const tt_buf *const buffer);

/*
tt_remrunner: Decrements tt_running by one (see tt_running). This should be called once a process that called tt_addrunner
        does not need the timetagger to be streaming new data. If the amount of runners is 0, a warning is shown, but nothing
        happens. This function is threadsafe.
    buffer: Pointer to buffer object

Example:
    tt_buf* buffer = tt_open(0);
    int a = tt_running(buffer);
    tt_addrunner(buffer);   //Adds one
    tt_remrunner(buffer);   //Subtracts one
    if (tt_running(buffer) == a) {
        //TRUE
    }
*/
TT_DEF_ void tt_remrunner(const tt_buf *const buffer);

//For simplicity
#define tt_removerunner tt_remrunner


//-------------------------------------------------------------------------------------------------------------
//Writing Data


/*
tt_add: Writes the datapoint to the end of the buffer, adding it. NOTE: The point MUST have timetag greater than or equal to
        any previous data point!
    buffer: The buffer object pointer
    channel: The channel number (starting from 0!)
    timetag: The time tag itself

Example:
    tt_buf* buffer = tt_create(0,5000000);
    uint8_t channel = 0;
    uint64_t tag = 9876543456;
    tt_add(buffer,channel, tag);    //Adds the datapoint to the buffer
*/
TT_DEF_ void tt_add(const tt_buf *const buffer, const uint8_t channel, const uint64_t timetag);

/*
tt_addarray: Writes array of datapoints to the end of the buffer, adding it. The amount of datapoints
        actually written is dependent on the buffer size - if a buffer is of size 10 and you have 100 points, then
        only the last 10 points are going to be written. NOTE: The array MUST be sorted! timetag[1] bust be less than
        or equal to timetag[2]! Otherwise everything breaks down. This is not checked, since that would require
        a traversal of the entire array.
    buffer: Buffer object pointer
    channels: Pointer to array of channels
    timetags: Pointer to the timetags corresponding to the given channels
    number: The number of datapoints to write

Example:
    tt_buf* buffer = tt_create(0,500000);
    uint8_t channels[3] = {0,1,0};
    uint64_t timetags[3] = {654563465,545345435353,546546546356464};    //Note ascending order
    tt_addarray(buffer,channels,timetags,3);
*/
TT_DEF_ void tt_addarray(const tt_buf *const buffer, const uint8_t *channels, const uint64_t *timetags,uint64_t datapoints);

/*
tt_addarray_offset: Has the same final effect as tt_addarray, but allows 2 extra operations: add/subtract and divide to the channel array,
        which is input as an array of integers. It also checks the first datapoint for overflow, and attempts to do a rough compensation if
        overflow is detected. It does not check the entire array.
    buffer: Buffer object of pointer
    channels: Pointer to integer array of channels
    timetags: Pointer to the timetags corresponding to the given channels
    number: The number of datapoints to write
    
    add: Number to add to each channel, 0 adds nothing
    div: Number to integer divide by, use 1 if do division needed. Do not pass in 0. Just don't.
    
The operation is as follows:
    real_channel = (channel+add)/div;
    
Example:
    tt_buf* buffer = tt_create(0,500000);
    uint8_t channel[3] = {5,7,5};               //The channels wanted are 0 1 0
    uint64_t timetags[3] = {50000,60000,70000};
    tt_addarray_offset(buffer,channels,timetags,3,-5,2); //(num-5)/2 is new channel, so 5->0, 7->1
*/
TT_DEF_ void tt_addarray_offset(const tt_buf *const buffer, const uint8_t *channels, const uint64_t *timetags, uint64_t datapoints, \
        const int add, const int div);

//-------------------------------------------------------------------------------------------------------------
//Reading Data

/*
tt_readarray: Reads points from startpoint to startpoint+datapoints into the given arrays. This also checks to make sure that the data was not overwritten
        by newer values and the like. If all goes well, it returns the number of datapoints read. if it fails to read, it returns 0.
    buffer: Pointer to buffer object
    startpoint: Which datapoint do you want to read? If it is not on the buffer, this function returns an error
    channel: Pointer to uint8_t which will be set to the channel value
    timetag: Pointer to uint64_t which will be set to the timetag value
    datapoints: How many datapoints do you want to read into the arrays?
    
    Returns:
        Number of datapoints read from buffer
        
Example:
    tt_buf* buffer = tt_open(0);
    uint8_t channels[100];
    uint64_t timetags[100];
    tt_readarray(buffer,tt_datapoints(buffer)-100,channels,timetags,100);   //Reads the most recent 100 datapoints handled by the buffer
    //Now timetags[99] and channels[99] correspond to the most recent datapoint
*/
TT_DEF_ uint64_t tt_readarray(const tt_buf *const buffer, uint64_t startpoint, uint8_t* channel, uint64_t *timetag, uint64_t datapoints);

//Reading one value is equivalent to reading an array with one value
#define tt_read(a,b,c,d) tt_readarray(a,b,c,d,1);

/*
tt_readchannel: Copies only points belonging to a certain channel to the given array. Behaves in the same way as tt_readarray,
but you input a channel number, and it returns the associated timetags. Returns the number of points written
    buffer: Pointer to buffer object
    startpoint; Which datapoint do you want to start at?
    channel: Which channel do you want?
    timetags: Pointer to array that will hold the timetags
    datapoints: How many datapoints do you want to read?

Example:
    tt_buf* buffer = tt_open(0);
    uint64_t timetags[100];
    tt_readchannel(buffer,tt_datapoints(buffer),0,timetags,100);    //Reads the most recent 100 datapoints of channel 0
*/
TT_DEF_ uint64_t tt_readchannel(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel, uint64_t* timetags, uint64_t datapoints);


//-------------------------------------------------------------------------------------------------------------
//Simplifying advanced interaction with Buffer

/*
tt_channelpoints: Given a starting point and number of datapoints forward to search, returns the number of data points
        from the given channel out of the datapoints searched
    buffer: Pointer to buffer object
    startpoint: Whch datapoint do you want to start at?
    channel: Which channel do you want?
    datapoints: How many datapoints do you want to search?

*/
TT_DEF_ uint64_t tt_channelpoints(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel, uint64_t datapoints);

/*
tt_bins2points: Given a starting point and the number of time bins *back* to search, it returns the number of datapoints
        which were within the timebin region of timebins from the recorded timetag of startpoint. (excluding the current tag)
    buffer: Pointer to buffer object
    startpoint: Which datapoint do you want to start at?
    timebins: How many time bins *back* do you want to search?
*/
TT_DEF_ uint64_t tt_bins2points(const tt_buf *const buffer, uint64_t startpoint, uint64_t timebins);

/*
tt_channeldist: Given a starting point and a number of channel datapoints you want to be included, returns the location
        of the datapoint at which the interval contains the necessary array
    buffer: Pointer to buffer object
    startpoint: At which point do you want to start at?
    channel: Which channel to search
    datapoints: The number of datapoints of the channel to search back
*/
TT_DEF_ uint64_t tt_channeldist(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel,uint64_t datapoints);

//-------------------------------------------------------------------------------------------------------------
//Timing Conversions
//There is a "resolution" defined in the buffer, which corresponds to the time in seconds for one "tick" in time tag.
//These functions allow for simplified conversions between seconds and time tagger clicks.

/*
tt_time2bin/tt_bin2time: Given a pointer to the buffer object, and a time in seconds or amount of time bins, if there is a 
        defined resolution for the buffer's data points, it does the necessary conversions from bins to time and back again.
        One thing to note is that this is NOT the same as "delayarray____". This does a straightforward conversion, while its
        delayarray big brothers do conversions for statistical functions
    buffer: Pointer to buffer object
    time/bins: An amount of time in seconds or time bins to be converted
*/
TT_DEF_ uint64_t tt_time2bin(const tt_buf *const buffer, double time);
TT_DEF_ double tt_bin2time(const tt_buf *const buffer, uint64_t bins);

/*
tt_delay(time2bin|bin2time): Given an array of channel delays (array of size channelnum), it converts the array to
        the other method to encode time (bins/time in seconds). If an array to write to is not included, then it allocates
        an array, which should be deallocated with tt_free.
    buffer: Pointer to buffer object
    time/bin: Array of delays to convert to the other type
    bin/time: Optional, array into which to convert.
    channelnum: The number of channels in the array (the channels are arbitrary order, just the delays matter)

One thing to note is that the bin delays are unsigned integers, while time delays are doubles, which can be negative. 
furthermore, the bin array is INVERTED. This means that the most negative delay in time will be the most positive delay
in time bins. While this may seem annoyingly arbitrary and counter-intuitive, it is there because calculations on the raw
data stream using this modified version of delay array become extremely simple.

Note that conversion and back is not necessarily going to give the same exact results
*/
TT_DEF_ uint64_t* tt_delaytime2bin(const tt_buf *const buffer, double* time, uint64_t* bin,int channelnum);
TT_DEF_ double* tt_delaybin2time(const tt_buf *const buffer, uint64_t* bin, double* time,int channelnum);

/*
tt_subtractreference: Given a time in bins, this function subtracts the time difference between reference time and the most
        recent datapoint. This means that if the time tagger was active, and just not receiving data pulses during a time period,
        then this function subtracts the active-but-no-pulse time period (in bins).
    buffer: Pointer to buffer object
    bins: An amount of time in time bins
*/
TT_DEF_ uint64_t tt_subtractreference(const tt_buf *const buffer, uint64_t bins);

//-------------------------------------------------------------------------------------------------------------
//Analysis of Data Stream

/*
tt_singles: Given a time to search back, returns number of single counts of the entire time tagger. If given an optional array, it
        also returns the singles for each channel as elements of the array
    buffer: Pointer to buffer object
    time: Time, in seconds to search back from "now", meaning the reference time.
    singles: (optional, use NULL if unused) Array of size channel-number, zeroed.
    
Note that tt_singles searches for singles from the reference time, rather than the most recent datapoint.


tt_rawsingles: Given the same arguments as tt_singles, except with time in time bins, and an additional parameter
        specifying the datanumber to start searching (back) from for singles.
    dataindex: The index of datapoint from which to search timebins for singles
*/
TT_DEF_ uint64_t tt_singles(const tt_buf *const buffer, double time,uint64_t* singles);
TT_DEF_ uint64_t tt_rawsingles(const tt_buf *const buffer, uint64_t timebins, uint64_t* singlesarray, uint64_t dataindex);



/*
tt_coincidences: Given a time to search back as well as a coincidence radius, both in seconds, returns a matrix of
        coincidences of each channel with each channel, with the diagonal being the number of singles in each channel. 
        If given an array of delays, it finds the coincidences while taking the delays into account.
        It returns Array of size channels*channels representing coincidences of each channel with each. Result
        will be a symmetric matrix. 
    buffer: Pointer to buffer object
    time: Time, in seconds to search back from "now", meaning the reference time.
    radius: Time, in seconds that pulses can differ and still be considered coincidences
    coincidenceMatrix: (optional, use NULL if unused) This allows one to allocate their own result matrix. If NULL, the
        function allocates one internally, and it must be freed after use.
    delayArray: (optional, use NULL if unused) Array of delays between channels, of size channels. This allows one to account for
        different fiber lengths and other delay-causing problems.

Note that tt_coincidences searches for tags from the reference time, rather than the most recent datapoint.

tt_rawcoincidences: Given the time-bin versions of the same arguments as tt_coincidences, and an additional parameter specifying
        the index at which to start searching back, calculates the coincidences. Used internally by tt_coincidences.
    dataindex: The index of datapoint from which to apply delays and start looking for coincidences.
*/
TT_DEF_ uint64_t* tt_coincidences(const tt_buf *const buffer, double time, double radius, uint64_t* coincidenceMatrix, double* delayArray);
TT_DEF_ uint64_t* tt_rawcoincidences(const tt_buf *const buffer, uint64_t timebins, uint64_t radius, uint64_t* coincidenceMatrix, uint64_t* delayArray, uint64_t dataindex);

//Simplified (faster) versions without delay handling (nd=nodelay)
TT_DEF_ uint64_t* tt_coincidences_nd(const tt_buf *const buffer, double time, double radius, uint64_t* coincidenceMatrix);
TT_DEF_ uint64_t* tt_rawcoincidences_nd(const tt_buf *const buffer, uint64_t timebins, uint64_t radius, uint64_t* coincidenceMatrix, uint64_t dataindex);

/*
tt_multicoincidences: Given a time back to search, a coincidence diameter (both in seconds), as well as an array of channels and optional delays,
        returns the number of times that *all* channels in channel array were within the same coincidence diameter. Basically a big AND for all of
        the channels passed in.
    buffer: Pointer to buffer object
    time: Time, in seconds to search back from "now", meaning the reference time.
    diameter: Time, in seconds within which all channels must have tags to be considered coincidences
    channels: Array of channel numbers which represent the channels that are to be checked for coincidences
    channelnum: The number of channels passed in
    delayArray: (optional, use NULL if unused) Array of delays between channels, of size channels. This allows one to account for
        different fibre lengths and other delay-causing problems. Used cleverly, this can also be more powerful.

Note that tt_multicoincidences searches for tags from the reference time, rather than the most recent datapoint.

tt_rawmulticoincidences: Given the time-bin versions of the same arguments as tt_multicoincidences, and an additional parameter specifying
        the index at which to start searching back, calculates the coincidences. Used internally by tt_multicoincidences.
    dataindex: The index of datapoint from which to apply delays and start looking for coincidences. 
*/
TT_DEF_ uint64_t tt_multicoincidences(const tt_buf *const buffer, double time, double diameter, uint8_t* channels, int channelnum, double* delayArray);
TT_DEF_ uint64_t tt_rawmulticoincidences(const tt_buf *const buffer, uint64_t timebins, uint64_t diameter, uint8_t* channels, uint8_t channelnum,
        uint64_t* delayArray, uint64_t dataindex);

/*
tt_correlate: Given a time back to search, a radius of correlation (both in seconds), as well as the number of "bins" and 2 channels with optional delays,
        returns a cross-correlation of the two channels. The radius of correlation is the amount to search on both sides of each datapoint in channel1, meaning
        that it is half of the total time diameter searched around a datapoint. To do an autocorrelation, just make both channels the same.
    buffer: Pointer to buffer object
    time: Time, in seconds to search back from "now", meaning the reference time.
    windowradius: Time, in seconds to search for correlated datapoints on each side of a channel1 datapoint
    bins: The number of buckets into which to divide the total correlation time
    channel1: The channel to use for correlation
    delay1: The delay of channel1. Use 0 if unknown/unneeded
    channel2: The channel to correlate channel1 with
    delay2: The delay of channel2. Use 0 if unknown/unneeded
    resultArray: (optional, use NULL if unused) allows one to input an already-allocated array of uint64_t with length bins. If NULL, allocates internally, meaning
        that you have to free it with tt_free.

Note that tt_correlate searches for tags from the reference time, rather than the most recent datapoint.

tt_rawcorrelate: Given the time-bin versions of the same arguments as tt_correlate, and an additional parameter specifying
        the index at which to start searching back, calculates the coincidences. Used internally by tt_correlate.
    dataindex: The index of datapoint from which to apply delays and start looking for correlations.  
*/
TT_DEF_ uint64_t* tt_correlate(const tt_buf *const buffer, double time, double windowradius, int bins, uint8_t channel1, double delay1, \
                                    uint8_t channel2,  double delay2,uint64_t* resultArray);
TT_DEF_ uint64_t* tt_rawcorrelate(const tt_buf *const buffer, uint64_t timebins, uint64_t windowradius, int bins, uint8_t channel1, uint64_t delay1, \
                                    uint8_t channel2, uint64_t delay2, uint64_t* resultArray, uint64_t dataindex);

//-------------------------------------------------------------------------------------------------------------
//Memory-related
/*
tt_free: Frees memory allocated by any of the tt_* functions. This is for use when this library is dynamically
        linked to a program, and the library and program use possibly different versions of a standard library.
    mem: Pointer to memory which is to be freed
*/
TT_DEF_ void tt_free(void *mem);

/*
tt_validateBuffer: Makes sure that the datapoints within a buffer are ordered correctly (increasing). Returns 1 if
        points are ordered (good), and returns 0 if there is a problem (BAD)
    buffer: Pointer to buffer to be validated
*/
TT_DEF_ int tt_validateBuffer(const tt_buf *const buffer);


//-------------------------------------------------------------------------------------------------------------
//Internally used macros - they allow for printing things in a consistent way across libTTAG.
//  These are made for internal use, and don't need to be used otherwise.

#ifdef TTAG_ENABLE_INTERNAL_MACROS

#include <stdio.h>

#define TT_ASSERT(as,r) if (!(as)) { \
    fprintf(stderr,"%i:%s: Assertion '%s' failed!\n",__LINE__,__FUNCTION__, #as ); \
    return r; \
}

#define TT_CHKERR(as,r,...) if (!(as)) { \
    fprintf(stderr,"%i:%s:ERROR: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}

#define TT_WARN(as,r,...) if (as) { \
    fprintf(stderr,"%i:%s:WARNING: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}

#define TT_ERROR(r,...) fprintf(stderr,"%i:%s: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\n"); \
    return r;

#define TT_PRINT(...) { printf("TT: "); printf(__VA_ARGS__); printf("\n"); }

#define TT_STDCHKERR(st,r) if ((int64_t)(st) == -1) { \
    fprintf(stderr,"%i:%s: '%s' Error: %s.\n",__LINE__,__FUNCTION__, #st,strerror(errno)); \
    r; \
}

#endif

#endif

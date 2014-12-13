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
Note: Documentation in ttag.h

    TODO:
        -   Add condition variable to runner to allow sleep-waiting (futex on linux and named
                mutex on windows
*/

#define TTAG_ENABLE_INTERNAL_MACROS //Enables macros used for simple output of warnings/errors/assertions
#include "ttag.h"

#include <stdlib.h> //malloc, much?
#include <string.h> //memcpy is needed here
#include <stdio.h>  //printing and error output
#include <errno.h>  //Checking if calls failed
#include <math.h>   //For round

//MSVC does not have round in math.h!
#ifndef round
#	define round(x) floor((x)+0.5)
#endif

//If this is a non-windows system, assume UNIX shared memory headers are available.
#ifndef _WIN32
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

/*
The memory map's structure is as follows:
    uint64_t  - Number of datapoints the buffer supports
    uint64_t  - The total number of datapoints that were ever written to the buffer
    uint64_t  - Reference time, a "now" value, to check how old data is
    uint64_t  - Same as total data, but increases in value during writes (allows for invalidating old data during write)
    float64   - Time, in seconds of one timetag click (resolution of time tagger)
    uint32_t  - Number of applications wanting timetagger to take data
    uint8_t   - Number of channels the timetagger supports/ channels that it will take data on
    uint8_t   - UNUSED
    uint16_t  - UNUSED
    uint64_t  - UNUSED
    uint64_t* - Array of times for the timetags
    uint8_t*  - Array of channels for the timetags

Note that memory alignment requires the unused space. This implementation assumes mmap'd memory is
aligned, meaning that all operations on the single variables are atomic on x86_64 (on 32bit 64 bit
variables might not be atomic, but synchronization should not be an issue anyways)
*/
#define TT_MAPSIZE(datapoints) (datapoints*(9)+8*7)

//Accessing data in header is done through these macros
#define TT_MAXDATA(b)       (b->map[0])
#define TT_TOTALDATA(b)     (b->map[1])
#define TT_REFERENCE(b)     (b->map[2])
#define TT_WRITEINDEX(b)    (b->map[3])
#define TT_RESOLUTION(b)    (((double*)b->map)[4])
#define TT_RUNNERS(b)       (*((uint32_t*)(b->map+5*sizeof(uint64_t))))
#define TT_CHANNELS(b)      (*((uint8_t*)(b->map+5*sizeof(uint64_t)+sizeof(uint32_t))))

//-------------------------------------------------------------------------------------------------------------
//Map Detection and buffer counting

//If the time tagger's 'driver' crashes on UNIX, the shared memory created is not freed. In this
//  case, the map needs to be manually deleted. These functions allow a fast simple way to do exactly
//  that.
#ifndef _WIN32
TT_DEF_ void tt_rawdeleteMap(char* mapname) {
    TT_ASSERT(mapname,);
    shm_unlink(mapname);
}
TT_DEF_ void tt_deleteMap(int mapnumber) {
    char mapname[TT_MAPMAX];           //String to hold the map name
    
    TT_ASSERT(mapnumber >= 0,);       //Make sure that the mapnumber is within range
    TT_ASSERT(mapnumber <= 99,);
    
    sprintf(mapname,TT_MAPNAMESTR,mapnumber);     //Set up the map name
    
    tt_rawdeleteMap(mapname);
}
#endif

//Checks if the given named map exists. This function is different on linux and windows, have different blocks
//  of code for the two operating systems.
TT_DEF_ int tt_mapExists(const char *const mapname) {
#ifdef _WIN32
    HANDLE descriptor;
    TT_ASSERT(mapname != NULL,0);

    //This function behaves differently on UNIX and Windows, so give
    //  a warning if possibly incorrect input is detected to make sure
    //  that the user is aware of this.
    TT_WARN(mapname[0]=='/',,"Mapname given with UNIX-style '/'!");
    

    //Attempt opening the map. If succeed, close it and return 1. If fail, return 0.
    descriptor = OpenFileMapping(FILE_MAP_WRITE,FALSE,mapname);
    
    if (descriptor) {
        CloseHandle(descriptor);
        return 1;
    } else {
        return 0;
    }

#else   //UNIX
    TT_ASSERT(mapname != NULL,0);
    
    //This function behaves differently on UNIX and Windows, so give
    //  a warning if possibly incorrect input is detected to make sure
    //  that the user is aware of this.
    TT_WARN(mapname[0]!='/',,"Given mapname does not start with '/'.");
    
    //Attempt opening the map. If succeed, close it and return 1. If fail, return 0.
    
    int descriptor = shm_open(mapname,O_RDONLY, TT_SHM_MODE);
    
    if (descriptor >= 0) {  //The descriptor was opened. Close it and return happily
        TT_STDCHKERR( close(descriptor) ,);
        return 1;
    } else if (errno != ENOENT) {   //If the error is NOT that the name does not exist
        TT_STDCHKERR( -1 ,);              //print the error description
    }
    
    return 0;
#endif

}

//Checks if the given map number exists
TT_DEF_ int tt_bufferExists(int mapnumber) {
    char mapname[TT_MAPMAX];           //String to hold the map name
    
    TT_ASSERT(mapnumber >= 0,0);       //Make sure that the mapnumber is within range
    TT_ASSERT(mapnumber <= 99,0);
    
    sprintf(mapname,TT_MAPNAMESTR,mapnumber);     //Set up the map name
    
    return tt_mapExists(mapname);
}

//Checks how many timeTagger buffers there are. It does not check all 100 possibilities,
//  but rather starts at 0 and keeps iterating until it does not find a buffer - and then it
//  checks some more to make sure that there are no more.
TT_DEF_ int tt_getBufferAmount() {
    int i = 0, lastSeen = 0;
    int bufferAmount = 0;
    
    //The maximum number of time taggers is 100, starting with 0
    for (i=0; i<=99; i++) {
        if (tt_bufferExists(i)) {
            lastSeen = i;
            bufferAmount++;
        } else if (lastSeen + 3 <= i) break;
    }
    
    return bufferAmount;
}

//Returns the next available buffer.
TT_DEF_ int tt_getNextFree() {
    int i = 0;
    
    //The maximum number of time taggers is 100, starting with 0
    for (i=0; i<=99; i++) {
        if (!tt_bufferExists(i)) return i;
    }
    
    //All buffers already exist
    return -1;
}

//Converts buffer index to map index
TT_DEF_ int tt_buffertomap(int buffernum) {
    int i = 0;
    int bufferAmount = -1;
    
    TT_ASSERT(buffernum >= 0,-1);
    TT_ASSERT(buffernum < tt_getBufferAmount(),-1);
    
    //The maximum number of time taggers is 100, starting with 0
    for (i=0; i<=99; i++) {
        if (tt_bufferExists(i)) {
            bufferAmount++;
            if (bufferAmount == buffernum) {
                return i;
            }
            TT_ASSERT(bufferAmount < buffernum,-1);    //This should never happen
        }
    }
    
    TT_ERROR(-1,"Unable to find the given index. Was a buffer just closed?");
}

//-------------------------------------------------------------------------------------------------------------
//Creating, opening and closing buffers.

//This creates a new buffer from a mapname
TT_DEF_ tt_buf* tt_rawcreate(const char *const mapname,const uint64_t datapoints) {
    tt_buf *map;
    
    TT_ASSERT(mapname != NULL, NULL);
    TT_ASSERT(strlen(mapname) < TT_MAPMAX, NULL);
    
    TT_WARN(datapoints <= 9999,,"Number of datapoints given is less than recommended minimum.");
    
    map = (tt_buf*) malloc(sizeof(tt_buf));
    TT_CHKERR(map,return NULL,"Memory allocation failed!");
    
#ifdef _WIN32
    {   //MSVC still clings to c89, so variable declarations are not fun.
        LARGE_INTEGER windows_int64;    //Microsoft version of 64 bit integer for use in the memory map API
        
        windows_int64.QuadPart = TT_MAPSIZE(datapoints);
        
        map->fileDescriptor = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE, \
                windows_int64.HighPart, windows_int64.LowPart, mapname);
        
        TT_CHKERR(map->fileDescriptor, free(map); return NULL,"Map creation failed!");
        
        //Get pointer associated with the file mapping
        map->map = (uint64_t*)MapViewOfFile(map->fileDescriptor,FILE_MAP_ALL_ACCESS,0,0, windows_int64.QuadPart);
        
        TT_CHKERR(map->map, CloseHandle(map->fileDescriptor); free(map); return NULL,"Memory mapping failed!");
    }
#else   //POSIX
    
    TT_STDCHKERR( map->fileDescriptor = shm_open(mapname,O_RDWR|O_CREAT|O_EXCL,TT_SHM_MODE) , free(map); return NULL);
    TT_STDCHKERR( ftruncate(map->fileDescriptor,TT_MAPSIZE(datapoints)),close(map->fileDescriptor); free(map);return NULL);
    TT_STDCHKERR( map->map = (uint64_t*)mmap(NULL,TT_MAPSIZE(datapoints),PROT_READ|PROT_WRITE,MAP_SHARED,map->fileDescriptor,0), \
            close(map->fileDescriptor); free(map); return NULL);
    //On POSIX, shared memory needs to be explicitly deleted, so make sure to delete it from the creator program.
    map->isCreator = 1;
    strcpy(map->name,mapname);
    
#endif

    //If the code gets here, creation of map was successful
    TT_WARN(((uint64_t)map->map)%8,,"Mapped memory not aligned - prepare for slow performance!");
    //Now, set the rest of the values necessary to make the buffer object valid
    map->timetag = map->map+TT_MAPSIZE(0);
    map->channel = (uint8_t*)map->timetag+sizeof(uint64_t)*datapoints;
    //The buffer object is complete. Now, in-buffer settings
    //  must be set to their default values.
    //The default values are chosen to be the most general possible settings.
    TT_MAXDATA(map) = datapoints;
    TT_TOTALDATA(map) = 0;
    TT_RESOLUTION(map) = NAN;
    TT_REFERENCE(map) = 0;
    TT_WRITEINDEX(map) = 0;
    TT_RUNNERS(map) = 0;
    TT_CHANNELS(map) = 255;
    return map;
}

//This is a wrapper around the tt_rawcreate function
TT_DEF_ tt_buf* tt_create(const int mapnumber,const uint64_t datapoints) {
    char mapname[TT_MAPMAX];           //String to hold the map name
    
    TT_ASSERT(mapnumber >= 0,0);       //Make sure that the mapnumber is within range
    TT_ASSERT(mapnumber <= 99,0);
    
    sprintf(mapname,TT_MAPNAMESTR,mapnumber);     //Set up the map name
    
    return tt_rawcreate(mapname,datapoints);
}

//Opens the given existing mapname
TT_DEF_ tt_buf* tt_rawopen(const char *const mapname) {
    tt_buf *map;
    
    TT_ASSERT(mapname != NULL, NULL);
    TT_ASSERT(strlen(mapname) < TT_MAPMAX, NULL);
    
    map = (tt_buf*)malloc(sizeof(tt_buf));
    
    TT_CHKERR(map,return NULL,"Memory allocation failed!");
    
#ifdef _WIN32

    map->fileDescriptor = OpenFileMapping(FILE_MAP_WRITE,FALSE,mapname);
    TT_CHKERR(map->fileDescriptor, free(map); return NULL,"Failed to open map!");
    
    //Maps the entire file
    map->map = (uint64_t*)MapViewOfFile(map->fileDescriptor,FILE_MAP_ALL_ACCESS,0,0,0);
    TT_CHKERR(map->map, CloseHandle(map->fileDescriptor); free(map); return NULL,"Memory mapping failed!");
    
#else   //UNIX

    TT_STDCHKERR( map->fileDescriptor = shm_open(mapname,O_RDWR,TT_SHM_MODE), free(map); return NULL);
    
    //The shared memory's size is necessary to map it
    struct stat st;
    TT_STDCHKERR(fstat(map->fileDescriptor,&st), close(map->fileDescriptor); free(map); return NULL);
    TT_CHKERR(st.st_size > TT_MAPSIZE(0), close(map->fileDescriptor); free(map); return NULL, \
            "Memory map sized incorrectly!");
    
    
    TT_STDCHKERR( map->map = (uint64_t*)mmap(NULL,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,map->fileDescriptor,0), \
            close(map->fileDescriptor); free(map); return NULL);
    
    //On POSIX, memory deletion code is needed, so label this as a "client"
    map->isCreator = 0;
    strcpy(map->name,mapname);
    
#endif

    //If the code gets here, opening of map was successful
    TT_WARN(((uint64_t)map->map)%8,,"Mapped memory not aligned - ops no longer atomic!");
    
    //Now, set the rest of the values necessary to make the buffer object valid
    map->timetag = map->map+TT_MAPSIZE(0);
    map->channel = (uint8_t*)map->timetag+sizeof(uint64_t)*TT_MAXDATA(map);
    
    return map;
}

//A wrapper around tt_rawopen
TT_DEF_ tt_buf* tt_open(const int mapnumber) {
    char mapname[TT_MAPMAX];           //String to hold the map name
    
    TT_ASSERT(mapnumber >= 0,0);       //Make sure that the mapnumber is within range
    TT_ASSERT(mapnumber <= 99,0);
    
    sprintf(mapname,TT_MAPNAMESTR,mapnumber);     //Set up the map name
    
    return tt_rawopen(mapname);
}

//Closes the buffer
TT_DEF_ void tt_close(const tt_buf *const buffer) {
    TT_ASSERT(buffer != NULL,)

#ifdef _WIN32
    //Windows automatically deletes the map once there are no readers
    UnmapViewOfFile(buffer->map);
    CloseHandle(buffer->fileDescriptor);
#else
    
    //The shared memory's size is necessary to unmap it
    struct stat st;
    
    //I REALLY hope that this won't give an error
    TT_STDCHKERR(fstat(buffer->fileDescriptor,&st),);
    
    
    //Close descriptors, and if there are no more readers left, unlink the shared memory
    TT_STDCHKERR(munmap((void*)buffer->map,st.st_size),);
    TT_STDCHKERR(close(buffer->fileDescriptor),);
    
    //On linux, shared memory is explicitly unlinked. The memory is unlinked by the creating process.
    if (buffer->isCreator) {
        TT_STDCHKERR(shm_unlink(buffer->name),);
    }
#endif
    
    //All things were cleaned, so free the buffer!
    free((void*)buffer);
}

//-------------------------------------------------------------------------------------------------------------
//Getting Buffer Information

//Get size in bytes of the buffer
TT_DEF_ uint64_t tt_buffersize(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,0);
    return TT_MAPSIZE(tt_maxdata(buffer));
}

TT_DEF_ uint64_t tt_maxdata(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,0);
    return TT_MAXDATA(buffer);
}

TT_DEF_ uint64_t tt_datapoints(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,0);
    return TT_TOTALDATA(buffer);
}

TT_DEF_ double tt_resolution(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,0);
    
    return TT_RESOLUTION(buffer);
}

TT_DEF_ int tt_channels(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,0);
    return 1+((int)TT_CHANNELS(buffer));    //The values of channel are 0-255 which correspond to 1-256 channels
}

TT_DEF_ uint64_t tt_reference(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL, 0);
    
    //If a timetag is greater than the reference, return that.
    if (tt_datanum(buffer) > 0 && tt_tag(buffer,tt_datanum(buffer)-1) > TT_REFERENCE(buffer)) {
        return tt_tag(buffer,tt_datanum(buffer)-1);
    }
    return TT_REFERENCE(buffer);
}

TT_DEF_ int tt_running(const tt_buf *const buffer) {
    TT_ASSERT(buffer != NULL, 0);
    return (int)TT_RUNNERS(buffer);
}

//-------------------------------------------------------------------------------------------------------------
//Setting Buffer Information

TT_DEF_ void tt_setdatapoints(const tt_buf *const buffer, uint64_t datapoints) {
    //TT_ASSERT(buffer != NULL, );
    
    TT_TOTALDATA(buffer) = datapoints;
}

TT_DEF_ void tt_setresolution(const tt_buf *const buffer, const double resolution) {
    //TT_ASSERT(buffer != NULL, );
    TT_ASSERT(resolution > 0.0 && resolution != INFINITY,); //The resolution must be a valid number
    
    TT_RESOLUTION(buffer) = resolution;
}

TT_DEF_ void tt_setchannels(const tt_buf *const buffer, const int channels) {
    //TT_ASSERT(buffer != NULL,);
    TT_ASSERT(channels>0 && channels<=256,);
    
    TT_CHANNELS(buffer) = (uint8_t)(channels-1);
}

TT_DEF_ void tt_setreference(const tt_buf *const buffer, const uint64_t ref) {
    //TT_ASSERT(buffer != NULL,);
    
    TT_REFERENCE(buffer) = ref;
}

TT_DEF_ void tt_setrunners(const tt_buf *const buffer, const int runners) {
    //TT_ASSERT(buffer != NULL,);
    TT_ASSERT(runners>=0,);    //The maximum number of runners is larger than int, so this function
                            //  does not give the full range possible of runners, but I don't expect
                            //  this being a problem anytime soon.
    
    TT_RUNNERS(buffer)= (uint32_t)runners;
}

TT_DEF_ void tt_addrunner(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,);
    
    //Adding runner must be an atomic operation, but compare-swap is very OS dependent
#ifdef _WIN32

    TT_WARN(TT_RUNNERS(buffer)== ~((uint32_t)0),return,"Maximum runner amount reached");
    
    //Note that this is a workaround for the functionality shown lower. There are cases
    //  where this will fail
    InterlockedIncrement(&TT_RUNNERS(buffer));
    
#elif defined(__GNUC__)
    
    uint32_t runners, newrunners;
    
    //Addition/subtraction while checking bounds is not atomic, so a workaround is needed
    do {
        runners = TT_RUNNERS(buffer);
        TT_WARN(runners== ~((uint32_t)0),return,"Maximum runner amount reached");
    
        newrunners = runners+1;
    } while (!__sync_bool_compare_and_swap(&TT_RUNNERS(buffer),runners,newrunners));

#else
#warning Atomic operations not found, using non-threadsafe alternative!

    //Make sure the next runner can fit
    TT_WARN(TT_RUNNERS(buffer)== ~((uint32_t)0),return,"Maximum runner amount reached");
    
    TT_RUNNERS(buffer)+=1;  //This is NOT a threadsafe operation!
    
#endif
}

TT_DEF_ void tt_remrunner(const tt_buf *const buffer) {
    //TT_ASSERT(buffer != NULL,);
    
#ifdef _WIN32
    //Check if it is 0
    TT_WARN(TT_RUNNERS(buffer)==0,return,"No runners are available to remove");
    
    if (TT_RUNNERS(buffer)==1) {
        TT_RUNNERS(buffer) = 0;     //Since subtraction does not have to be atomic, the set to 0 is safer
    } else {
        InterlockedDecrement(&TT_RUNNERS(buffer));
    }
    
#elif defined(__GNUC__)

    uint32_t runners, newrunners;
    
    //Addition/subtraction while checking bounds is not atomic, so a workaround is needed
    do {
        runners = TT_RUNNERS(buffer);
        TT_WARN(runners==0,return,"No runners available to remove.");
        
        newrunners = runners -1;
    } while (!__sync_bool_compare_and_swap(&TT_RUNNERS(buffer),runners,newrunners));

#else
#warning Atomic operations not found, using non-threadsafe alternative!

    //Check if it is 0
    TT_WARN(TT_RUNNERS(buffer)==0,return,"No runners are available to remove");
    
    if (TT_RUNNERS(buffer)==1) {
        TT_RUNNERS(buffer) = 0;     //Since subtraction does not have to be atomic, the set to 0 is safer
    } else {
        TT_RUNNERS(buffer) -=1;
    }

#endif
}



//-------------------------------------------------------------------------------------------------------------
//Writing Data

TT_DEF_ void tt_add(const tt_buf *const buffer, const uint8_t channel, const uint64_t timetag) {
    //TT_ASSERT(buffer != NULL,);
    
    //Note that this assumes that tt_datanum(buffer) won't change between operations, which
    //  means that only one thread is writing data at a time - since timetags must be ordered,
    //  that should always be the case
    
    ++tt_writeindex(buffer);

    tt_channel(buffer,tt_datanum(buffer)) = channel;
    tt_tag(buffer,tt_datanum(buffer)) = timetag;
    
    ++tt_datanum(buffer);
}

TT_DEF_ void tt_addarray(const tt_buf *const buffer, const uint8_t *channels, const uint64_t *timetags,uint64_t datapoints) {
    uint64_t curloc;    //The current location in the buffer
    
    //TT_ASSERT(buffer != NULL,);
    TT_ASSERT(channels != NULL,);
    TT_ASSERT(timetags != NULL,);
    
    //Make the memory being written invalid for read
    TT_ASSERT(tt_writeindex(buffer)==tt_datanum(buffer),);
    tt_writeindex(buffer)+=datapoints;

    TT_WARN(datapoints > TT_MAXDATA(buffer), \
        channels = &(channels[datapoints-TT_MAXDATA(buffer)]); \
        timetags = &(timetags[datapoints-TT_MAXDATA(buffer)]); \
        tt_datanum(buffer) += datapoints - TT_MAXDATA(buffer); \
        datapoints = TT_MAXDATA(buffer); \
    ,"Datapoint number exceeds buffer size");

    //The current location is just modding
    curloc = tt_datanum(buffer)%TT_MAXDATA(buffer);
    
    if (!((curloc+datapoints)/TT_MAXDATA(buffer))) {
        //There is no need for the data to rotate to the beginning of the buffer
        memcpy(&tt_channel(buffer,tt_datanum(buffer)),channels,datapoints*sizeof(uint8_t));
        memcpy(&tt_tag(buffer,tt_datanum(buffer)),timetags,datapoints*sizeof(uint64_t));
    } else {
        //The array needs to wrap around to the beginning of the buffer
        memcpy(&tt_channel(buffer,tt_datanum(buffer)),channels,TT_MAXDATA(buffer)-curloc);
        memcpy(&tt_tag(buffer,tt_datanum(buffer)),timetags,sizeof(uint64_t)*(TT_MAXDATA(buffer)-curloc));
        
        memcpy(&tt_channel(buffer,0),&(channels[TT_MAXDATA(buffer)-curloc]),datapoints+curloc-TT_MAXDATA(buffer));
        memcpy(&tt_tag(buffer,0),&(timetags[TT_MAXDATA(buffer)-curloc]),sizeof(uint64_t)*(datapoints+curloc-TT_MAXDATA(buffer)));
    }
    
    //Update the amount of data - the original write index
    tt_datanum(buffer)=tt_writeindex(buffer);
}


TT_DEF_ void tt_addarray_offset(const tt_buf *const buffer, const uint8_t *channels, const uint64_t *timetags, uint64_t datapoints, const int add, const int div) {
    uint8_t* channel;   //A speedup-variable
    uint64_t* tag;		//A speedup variable
    uint64_t curloc;    //The current location in the buffer
    uint64_t addon = 0;
    uint64_t i;
    
    //TT_ASSERT(buffer != NULL,);
    TT_ASSERT(channels != NULL,);
    TT_ASSERT(timetags != NULL,);
    TT_ASSERT(div != 0,);
    
    //Make the memory being written invalid for read
    TT_ASSERT(tt_writeindex(buffer)==tt_datanum(buffer),);
    tt_writeindex(buffer)+=datapoints;

    TT_WARN(datapoints > TT_MAXDATA(buffer), \
        channels = &(channels[datapoints-TT_MAXDATA(buffer)]); \
        timetags = &(timetags[datapoints-TT_MAXDATA(buffer)]); \
        tt_datanum(buffer) += datapoints - TT_MAXDATA(buffer); \
        datapoints = TT_MAXDATA(buffer); \
    ,"Datapoint number exceeds buffer size");

    //The current location is just modding
    curloc = tt_datanum(buffer)%TT_MAXDATA(buffer);
    
    //Some datasets could have reset times. If we detect this, add on an offset
    TT_WARN(datapoints && tt_reference(buffer) > timetags[0], addon = tt_reference(buffer);,\
        "Time stamp less than reference - adding offset to array");

    if (!((curloc+datapoints)/TT_MAXDATA(buffer))) {
        //There is no need for the data to rotate to the beginning of the buffer
        if (!addon) {
            memcpy(&tt_tag(buffer,tt_datanum(buffer)),timetags,datapoints*sizeof(uint64_t));
        } else {
            tag = &tt_tag(buffer,tt_datanum(buffer));
        
            for (i=0;i<datapoints;i++) {
                tag[i] = timetags[i]+addon;
            }
        }

        channel = &tt_channel(buffer,tt_datanum(buffer));
        
        for (i=0;i<datapoints;i++) {
            channel[i] = (uint8_t)((channels[i]+add)/div);
        }

        
    } else {
        //The array needs to wrap around to the beginning of the buffer
        if (!addon) {
            memcpy(&tt_tag(buffer,tt_datanum(buffer)),timetags,sizeof(uint64_t)*(TT_MAXDATA(buffer)-curloc));
            memcpy(&tt_tag(buffer,0),&(timetags[TT_MAXDATA(buffer)-curloc]),sizeof(uint64_t)*(datapoints+curloc-TT_MAXDATA(buffer)));
        } else {
            tag = &tt_tag(buffer,tt_datanum(buffer));
            for (i=0; i< TT_MAXDATA(buffer)-curloc;i++) {
                tag[i] = timetags[i]+addon;
            }
        
            tag = &tt_tag(buffer,0);
            timetags = &(timetags[TT_MAXDATA(buffer)-curloc]);
            for (i=0; i < datapoints+curloc-TT_MAXDATA(buffer); i++) {
                tag[i] = timetags[i]+addon;
            }
        }

        channel = &tt_channel(buffer,tt_datanum(buffer));
        for (i=0; i< TT_MAXDATA(buffer)-curloc;i++) {
            channel[i] = (uint8_t)((channels[i]+add)/div);
        }
        
        channel = &tt_channel(buffer,0);
        channels = &(channels[TT_MAXDATA(buffer)-curloc]);
        for (i=0; i < datapoints+curloc-TT_MAXDATA(buffer); i++) {
            channel[i] = (uint8_t)((channels[i]+add)/div);
        }
    }
    
    //Update the amount of data - the original write index
    tt_datanum(buffer)=tt_writeindex(buffer);
}

//-------------------------------------------------------------------------------------------------------------
//Reading Data

TT_DEF_ uint64_t tt_readarray(const tt_buf *const buffer,uint64_t startpoint, uint8_t* channel, uint64_t *timetag, uint64_t datapoints) {
    uint64_t curloc;    //The current location in the buffer
    
    //Make sure that the request is valid
    //TT_ASSERT(buffer != NULL,0);
    TT_ASSERT(channel || timetag,0);    //Make sure there was some sort of input array passed
    TT_ASSERT(startpoint < tt_datanum(buffer),0);
    TT_CHKERR(startpoint >= tt_minindex(buffer),return 0,"Data requested no longer in buffer!");
    TT_WARN(startpoint+datapoints > tt_datanum(buffer),datapoints = tt_datanum(buffer)-startpoint, \
        "Requested more than buffer. Reading only to buffer size.");
    
    curloc = startpoint%TT_MAXDATA(buffer);
    
    //Read to the array
    if (!((curloc+datapoints)/TT_MAXDATA(buffer))) {
        //There is no need for the data to rotate to the beginning of the buffer
        if (channel)    memcpy(channel,&tt_channel(buffer,startpoint),datapoints);
        if (timetag)    memcpy(timetag,&tt_tag(buffer,startpoint),datapoints*sizeof(uint64_t));
        
    } else {
        //The array needs to wrap around to the beginning of the buffer
        if (channel) {
            memcpy(channel,&tt_channel(buffer,startpoint),TT_MAXDATA(buffer)-curloc);
            memcpy(&channel[TT_MAXDATA(buffer)-curloc],&tt_channel(buffer,0),datapoints+curloc-TT_MAXDATA(buffer));
        }
        if (timetag) {
            memcpy(timetag,&tt_tag(buffer,startpoint),sizeof(uint64_t)*(TT_MAXDATA(buffer)-curloc));
            memcpy(&(timetag[TT_MAXDATA(buffer)-curloc]),&tt_tag(buffer,0),sizeof(uint64_t)*(datapoints+curloc-TT_MAXDATA(buffer)));
        }
    }
    
    return datapoints;
}


TT_DEF_ uint64_t tt_readchannel(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel, uint64_t* timetags, uint64_t datapoints) {
    uint64_t number = 0;
    
    //TT_ASSERT(buffer != NULL,0);
    TT_ASSERT(timetags != NULL,0);
    TT_ASSERT(datapoints > 0,0);
    TT_ASSERT(startpoint < tt_datanum(buffer),0);
    TT_ASSERT(startpoint >= tt_minindex(buffer),0);

    TT_WARN(startpoint+datapoints > tt_datanum(buffer),datapoints = tt_datanum(buffer)-startpoint, \
        "Requested more than buffer. Reading only to buffer size.");
    
    for (;number < datapoints && startpoint < tt_datanum(buffer) ;startpoint++) {
        if (tt_channel(buffer,startpoint) == channel) {
            timetags[number] = tt_tag(buffer,startpoint);
            number++;
        }
    }
    
    return number;
}


TT_DEF_ uint64_t tt_channelpoints(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel, uint64_t datapoints) {
    uint64_t number = 0;
    
    //TT_ASSERT(buffer != NULL,0);
    TT_ASSERT(startpoint < tt_datanum(buffer),0);
    TT_ASSERT(startpoint >= tt_minindex(buffer),0);
    TT_ASSERT(datapoints > 0,0);
    TT_WARN(startpoint+datapoints > tt_datanum(buffer),datapoints = tt_datanum(buffer)-startpoint, \
        "Requested more than buffer. Reading only to buffer size.");
    
    for (;number < datapoints && startpoint < tt_datanum(buffer) ;startpoint++) {
        if (tt_channel(buffer,startpoint) == channel) number++;
    }
    
    return number;
}

TT_DEF_ uint64_t tt_channeldist(const tt_buf *const buffer, uint64_t startpoint, uint8_t channel,uint64_t datapoints) {
    
    //TT_ASSERT(buffer != NULL, 0);
    TT_ASSERT(startpoint < tt_datanum(buffer),0);
    TT_ASSERT(datapoints > 0, 0);
    TT_ASSERT(startpoint >= tt_minindex(buffer),0);

    TT_WARN(startpoint < datapoints,datapoints = startpoint, \
        "Requested more than buffer. Reading only to buffer size.");

    for (;datapoints > 0 && startpoint >= tt_minindex(buffer) && startpoint > 0;startpoint--) {
       if (tt_channel(buffer,startpoint)==channel) datapoints--;
    }

    return startpoint+1;
}

//This function assumes ordered input!
TT_DEF_ uint64_t tt_bins2points(const tt_buf *const buffer, uint64_t startpoint, uint64_t timebins) {
    uint64_t left;
    uint64_t right;
    uint64_t point;
    uint64_t temp;
    
    //TT_ASSERT(buffer != NULL, 0);
    TT_ASSERT(startpoint < tt_datanum(buffer),0);              //Make sure that the startpoint is within bounds of array
    TT_ASSERT(startpoint >= tt_minindex(buffer),0);
    
    //This can be done in log(n) time, so let's do it in log(n) time!
    
    left = tt_minindex(buffer);
    right = startpoint;
    
    //If the requested time is greater or equal to the total time, return all the buffered datapoints.
    TT_WARN(timebins+tt_tag(buffer,tt_minindex(buffer)) >= tt_tag(buffer,right),return right - tt_minindex(buffer);,"Requested time is more than entire buffer, returning buffer size");
    
    point = tt_tag(buffer,startpoint)-timebins;
    left = tt_minindex(buffer);
    TT_ASSERT(left <= right,0);	//During data acquisition, this is not necessarily true

    //I'd rather not use recursion here, since I want this to be self-contained
    while (right-left > 1) {
        temp = (left+right)/2;
        if (tt_tag(buffer,temp) < point) {
            left = temp+1;
        } else {
            right = temp;
        }
    }
    
    if (tt_tag(buffer,left) < point) return startpoint - right;
    return startpoint - left;
}

//-------------------------------------------------------------------------------------------------------------
//Timing Conversions

TT_DEF_ uint64_t tt_time2bin(const tt_buf *const buffer, double time) {
    //TT_ASSERT(buffer,0);
    TT_ASSERT(!isnan(tt_resolution(buffer)) && tt_resolution(buffer)!=0.0,0);
    
    //Rounding is necessary to get exact answers, since we want the best answer
    //  possible, not just a "close" answer
    return (uint64_t)round(time/tt_resolution(buffer));
}

TT_DEF_ double tt_bin2time(const tt_buf *const buffer, uint64_t bins) {
    //TT_ASSERT(buffer,0);
    TT_ASSERT(!isnan(tt_resolution(buffer)) && tt_resolution(buffer)!=0.0,0);
    
    return ((double)bins)*tt_resolution(buffer);
}

TT_DEF_ uint64_t tt_subtractreference(const tt_buf *const buffer, uint64_t bins) {
    //TT_ASSERT(buffer,bins);
    TT_ASSERT(tt_datanum(buffer),bins);
    
    if (tt_reference(buffer) > tt_tag(buffer,tt_datanum(buffer)-1)) {
        if (tt_reference(buffer)-tt_tag(buffer,tt_datanum(buffer)-1) < bins) {
            bins -= tt_reference(buffer)-tt_tag(buffer,tt_datanum(buffer)-1);
        } else return 0;    //There are no datapoints within the time
    }
    return bins;
}

TT_DEF_ uint64_t* tt_delaytime2bin(const tt_buf *const buffer, double* time, uint64_t* bin, int channelnum) {
    int i;
    int64_t offset;
    int64_t tmp;
    //TT_ASSERT(buffer,NULL);
    TT_ASSERT(!isnan(tt_resolution(buffer)) && tt_resolution(buffer)!=0.0,0);
    TT_ASSERT(channelnum>=1,0);
    
    //If the bin array is not allocated, allocate one
    if (!bin) {
        bin = (uint64_t*)malloc(sizeof(uint64_t)*channelnum);
        TT_ASSERT(bin!=NULL,NULL);
    }
    
    //Find offset that makes the smallest number in delays 0 (to fix possible negatives)
    offset= (int64_t)round(time[0]/tt_resolution(buffer));
    for (i=channelnum-1;i>=1;i--) {
        tmp = (int64_t)round(time[i]/tt_resolution(buffer));
        if (tmp > offset) offset = tmp;
    }
    
    
    for (i=channelnum-1;i>=0;i--) {
        bin[i] = (uint64_t)(offset - (int64_t)round(time[i]/tt_resolution(buffer)));
    }
    
    return bin;
}

TT_DEF_ double* tt_delaybin2time(const tt_buf *const buffer, uint64_t* bin, double* time,int channelnum) {
    int i;
    
    //TT_ASSERT(buffer,NULL);
    TT_ASSERT(!isnan(tt_resolution(buffer)) && tt_resolution(buffer)!=0.0,0);
    TT_ASSERT(channelnum>=1,0);
    
    //If the time array is not allocated, allocate one
    if (!time) {
        time = (double*)malloc(sizeof(double)*channelnum);
        TT_ASSERT(time,NULL);
    }
    
    for (i=channelnum-1;i>=0;i--) {
        time[i] = -((double)bin[i])*tt_resolution(buffer);
    }
    
    return time;
}

//-------------------------------------------------------------------------------------------------------------
//Memory-related

//A simple free wrapper, which allows functions to return allocated memory if ttag is compiled as 
//  a shared library
TT_DEF_  void tt_free(void *mem) {
    free(mem);
}

//A simple function which checks the validity of data on the buffer - meaning that it cycles throug the buffer to make
//  sure that the time tags are ordered. TT_WARNING: If datapoints are being added during a validate, it is possible that
//  it would say that it is invalid, due to memory being overwritten with new data.
TT_DEF_ int tt_validateBuffer(const tt_buf *const buffer) {
    uint64_t datapoints;
    uint64_t i;
    //TT_ASSERT(buffer != NULL, 0);
    
    //Find the number of datapoints currently in the buffer
    datapoints = tt_datapoints(buffer);
    if (datapoints < 2) return 1;  //If the buffer is empty, or has only one value, then it is valid
    
    //Make sure that the correct number of datapoints is chosen
    if (datapoints > tt_maxdata(buffer)) datapoints = tt_maxdata(buffer);

    //Start from the oldest datapoint in buffer and move forward to the most recent datapoint
    for (i=tt_datanum(buffer)-datapoints; i < tt_datanum(buffer)-1 ; i++) {
        if (tt_tag(buffer,i) > tt_tag(buffer,i+1)) {
            //Invalid buffer
            return 0;
        }
    }
    //If it is valid, return 1 (true)
    return 1;
}


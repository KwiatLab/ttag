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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <fstream>
	using namespace std;

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
	namespace po = boost::program_options;

#include "tdcbase.h"	//Include the quTau library
#include "ttag.h"		//Includes the timetagger buffer


#define QU_VERSION 0.1f	//The program's version

//This is a global variable, which is set to 0 if ctrl+c is pressed or termination signal is sent. It allows a clean exit
int volatile runAcquisition = 1;	

#define ASSERT(as,r) if (!(as)) { \
    fprintf(stderr,"%i:%s: Assertion '%s' failed!\n",__LINE__,__FUNCTION__, #as ); \
	system("pause"); \
    return r; \
}
#define WARN(as,r,...) if (as) { \
    fprintf(stderr,"%i:%s:WARNING: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); \
    fprintf(stderr,"\n"); \
    r; \
}

#define ERROR(r,...) fprintf(stderr,"%i:%s: ",__LINE__,__FUNCTION__); \
    fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\n"); \
	system("pause"); \
    return r;


/* Print out return codes */
static const char * printRc( int rc )
{
  switch ( rc ) {
  case TDC_Ok:           return "Success";
  case TDC_Error:        return "Unspecified error";
  case TDC_Timeout:      return "Receive timed out";
  case TDC_NotConnected: return "No connection was established";
  case TDC_DriverError:  return "Error accessing the USB driver";
  case TDC_DeviceLocked: return "Can't connect device because already in use";
  case TDC_Unknown:      return "Unknown error";
  case TDC_NoDevice:     return "Invalid device number used in call";
  case TDC_OutOfRange:   return "Parameter in fct. call is out of range";
  case TDC_CantOpen:     return "Can't open specified file";
  default:                return "????";
  }
}


static void checkRc( const char * fctname, int rc )
{
  if ( rc ) {
    printf( ">>> %s: %s\n", fctname, printRc( rc ) );
    exit( 1 );
  }
}

//Upon catching a ctrl+c it exits
static void exitsignal(int signal) {
    printf( "> Stopping quTau                         \n" );
    runAcquisition = 0; //Tell it to stop getting data
}

int main( int argc, char ** argv )
{
	tt_buf* buffer;				//The libTTag buffer: tags are written to it
	Int64 timestamps[10000];	//Timestamp buffer
	Int8  channels[10000];		//Channel buffer
	int tagcount;				//The number of tags
	int datalost=0;				//Whether there was an overflow
	int overflownum = 0;		//number of overflow

	//Variables that can be set by the user through command line options or configuration file
	uint64_t buffersize;		//The size of libTTag's buffer
	string configfilename;		//Name of configuration file from which to load settings
	bool autostart;				//If true, starts taking data immediately (adds a runner)
	int	qubuffer;			//quTau internal buffer. Bigger is better. Up to 10k.
	int deviceID;				//the quTAU device ID. Allows to choose a specific time tagger

	//These are command-line only options
    po::options_description genopt("Generic Options");
	genopt.add_options()
		("help,h","Show usage and help message")
		("config,c",po::value<string>(&configfilename)->default_value("qutau.qucfg"),"Set configuration file to use")
    ;
	po::positional_options_description pos;
	pos.add("config",1);
	
	//These are options available from both command line and configuration file
	po::options_description ttopt("Time Tagger Options");
	ttopt.add_options()
		("buffer,b",po::value<uint64_t>(&buffersize)->default_value(100000000),"Choose size of the time tag buffer")
		("autostart",po::value<bool>(&autostart)->default_value(false),"If enabled, starts taking data immediately (adds runner)")
		("qubuffer",po::value<int>(&qubuffer)->default_value(10000),"The size, in time stamps of the quTau's internal buffer, up to 10k. Bigger is better.")
		("ttid",po::value<int>(&deviceID)->default_value(-1),"ID of time tagger to open. Opens any by default.")
    ;
	
	po::options_description cmdline;
	cmdline.add(genopt).add(ttopt);

	po::options_description configfile;
	configfile.add(ttopt);

	po::variables_map vm;

	//Read command line options
	try {
		po::store(po::command_line_parser(argc,argv).options(cmdline).positional(pos).run(),vm);
		po::notify(vm);
	} catch (exception& e) {
		cout << "Error: " << e.what() << endl;
		return 1;
	} catch (...) {
		cout << "Error parsing command line..." << endl;
		return 1;
	}

	//Read configuration file
	try {
		ifstream cfile;
		cfile.open(configfilename.c_str());
		po::store(po::parse_config_file(cfile,configfile,true),vm);
		po::notify(vm);
	} catch (exception& e) {
		cout << "Config File: " << e.what() << endl;
		return 1;
	} catch (...) {
		cout << "Error parsing config file" << endl;
		return 1;
	}

	if (vm.count("help")) {
		std::cout << cmdline << "\n";
		return 2;
	}

	

	cout << "quTAU tag streamer version " << QU_VERSION << endl << endl;

	//Make sure all settings are within bounds
	ASSERT(buffersize >= 50000000,-5);
	WARN(buffersize < 100000000,,"The buffer size chosen is a bit small. Will try to continue.\n");
	WARN(qubuffer > 10000,qubuffer = 10000,"Agilent buffer too large. Adapting to maximum accepted value (%i).",10000);
	
	//Print out settings
	cout << "Buffer Size:         " << buffersize << endl;
	cout << "quTau Buffer:        " << qubuffer << endl;
	cout << "AutoStart:           " << (autostart?"true":"false") << endl;
	cout << "Resolution:          " << TDC_getTimebase() << endl;
	cout << "Device ID:           " << deviceID << endl;

	//Set exit signals
	if (signal(SIGINT,exitsignal) == SIG_ERR) {
        ERROR(-2,"Failed to set SIGINT!");
    }
    if (signal(SIGTERM,exitsignal) == SIG_ERR) {
        ERROR(-2,"Failed to set SIGTERM!");
    }

	cout << endl;
	cout << "> Initializing" << endl;

	//Initialize time tagger
	ASSERT(!TDC_init(deviceID),-1);						//Initialize the time tagger
	ASSERT(!TDC_enableChannels( 0xff ),-2);				//Use all 8 channels
	ASSERT(!TDC_setTimestampBufferSize(qubuffer),-3);	//Initialize timestamp buffer
	ASSERT(!TDC_freezeBuffers(0),-4);					//Make sure everything is unfrozen

	//Create the time tagger's buffer
	int freebuffer = tt_getNextFree();
	buffer = tt_create(freebuffer,buffersize);
	ASSERT(buffer,-4);

	//Set the time tagger's resolution
	tt_setresolution(buffer,TDC_getTimebase());
	tt_setchannels(buffer,8);
	cout << "> Using buffer " << freebuffer << endl;

	//If it is to start automatically, add a runner
	if (autostart) {
		tt_addrunner(buffer);
	}

	while (runAcquisition) {
		while (!tt_running(buffer)) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(200));
		}
		printf("> Started Acquisition                              \n");
		while (tt_running(buffer) && runAcquisition) {
			TDC_getDataLost(&datalost);
			if (datalost) {
				printf(">> OVERFLOW %i                          \n",overflownum++);
			}

			TDC_getLastTimestamps( 1, timestamps, channels, &tagcount );
			tt_addarray(buffer,(uint8_t*)channels,(uint64_t*)timestamps,(uint64_t)tagcount);
			
			printf(" Reading: %llu                       \r",tt_datapoints(buffer));
		}
		printf("> Stopped Acquisition                              \n");
	}

	printf("> Cleaning Up...                    \n" );
	tt_close(buffer);
	TDC_deInit();
	return 0;
}

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

#include "CTimeTag.h"	//Include the UQD library
#include "ttag.h"		//Includes the timetagger buffer
	using namespace TimeTag;

#define UQD_VERSION 0.1f	//The program's version

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

//Upon catching a ctrl+c it exits
static void exitsignal(int signal) {
    printf( "> Stopping Time Tagger                      \n" );
    runAcquisition = 0; //Tell it to stop getting data
}

int main(int argc, char** argv) {
	tt_buf* buffer;				//The libTTag buffer: tags are written to it
	ChannelType *channelarray;	//The array for channel
	TimeType *tagarray;			//The array for time
	CTimeTag tagger;			//The time tagger object

	//Variables that can be set by the user through command line options or configuration file
	uint64_t buffersize;		//The size of libTTag's buffer
	string configfilename;		//Name of configuration file from which to load settings
	bool autostart;				//If true, starts taking data immediately (adds a runner)
	bool reference;				//The time taggers have a 10MHz reference input. This boolean chooses whether or not to use it. true=use, false=use internal clock
	int deviceID;				//The number of time tagger to open. Default is 1
	float channelvoltage[16];	//The threshold voltage for each channel
	bool channeledge[16];		//The edge to trigger each channel: true=rising, false=falling
	int ledbrightness;			//The LED brightness
	//These are command-line only options
    po::options_description genopt("Generic Options");
	genopt.add_options()
		("help,h","Show usage and help message")
		("config,c",po::value<string>(&configfilename)->default_value("uqd.uqcfg"),"Set configuration file to use")
    ;
	po::positional_options_description pos;
	pos.add("config",1);

	//These are options available from both command line and configuration file
	po::options_description ttopt("Time Tagger Options");
	ttopt.add_options()
		("buffer,b",po::value<uint64_t>(&buffersize)->default_value(100000000),"Choose size of the time tag buffer")
		("reference,r",po::value<bool>(&reference)->default_value(false),"Whether or not to use the 10MHz reference input")
		("autostart",po::value<bool>(&autostart)->default_value(false),"If enabled, starts taking data immediately (adds runner)")
		("ttid",po::value<int>(&deviceID)->default_value(1),"Number of time tagger to open.")
		("led",po::value<int>(&ledbrightness)->default_value(30),"Brightness of LED, as an integer percentage.")
    ;

	po::options_description copt("Channel Options");
	copt.add_options()
		("c1v",po::value<float>(&(channelvoltage[0]))->default_value(1.0),"Threshold voltage for channel 1")
		("c2v",po::value<float>(&(channelvoltage[1]))->default_value(1.0),"Threshold voltage for channel 2")
		("c3v",po::value<float>(&(channelvoltage[2]))->default_value(1.0),"Threshold voltage for channel 3")
		("c4v",po::value<float>(&(channelvoltage[3]))->default_value(1.0),"Threshold voltage for channel 4")
		("c5v",po::value<float>(&(channelvoltage[4]))->default_value(1.0),"Threshold voltage for channel 5")
		("c6v",po::value<float>(&(channelvoltage[5]))->default_value(1.0),"Threshold voltage for channel 6")
		("c7v",po::value<float>(&(channelvoltage[6]))->default_value(1.0),"Threshold voltage for channel 7")
		("c8v",po::value<float>(&(channelvoltage[7]))->default_value(1.0),"Threshold voltage for channel 8")
		("c9v",po::value<float>(&(channelvoltage[8]))->default_value(1.0),"Threshold voltage for channel 9")
		("c10v",po::value<float>(&(channelvoltage[9]))->default_value(1.0),"Threshold voltage for channel 10")
		("c11v",po::value<float>(&(channelvoltage[10]))->default_value(1.0),"Threshold voltage for channel 11")
		("c12v",po::value<float>(&(channelvoltage[11]))->default_value(1.0),"Threshold voltage for channel 12")
		("c13v",po::value<float>(&(channelvoltage[12]))->default_value(1.0),"Threshold voltage for channel 13")
		("c14v",po::value<float>(&(channelvoltage[13]))->default_value(1.0),"Threshold voltage for channel 14")
		("c15v",po::value<float>(&(channelvoltage[14]))->default_value(1.0),"Threshold voltage for channel 15")
		("c16v",po::value<float>(&(channelvoltage[15]))->default_value(1.0),"Threshold voltage for channel 16")

		("c1e",po::value<bool>(&(channeledge[0]))->default_value(true),"Edge triggering for channel 1 (1=rising,0=falling)")
		("c2e",po::value<bool>(&(channeledge[1]))->default_value(true),"Edge triggering for channel 2")
		("c3e",po::value<bool>(&(channeledge[2]))->default_value(true),"Edge triggering for channel 3")
		("c4e",po::value<bool>(&(channeledge[3]))->default_value(true),"Edge triggering for channel 4")
		("c5e",po::value<bool>(&(channeledge[4]))->default_value(true),"Edge triggering for channel 5")
		("c6e",po::value<bool>(&(channeledge[5]))->default_value(true),"Edge triggering for channel 6")
		("c7e",po::value<bool>(&(channeledge[6]))->default_value(true),"Edge triggering for channel 7")
		("c8e",po::value<bool>(&(channeledge[7]))->default_value(true),"Edge triggering for channel 8")
		("c9e",po::value<bool>(&(channeledge[8]))->default_value(true),"Edge triggering for channel 9")
		("c10e",po::value<bool>(&(channeledge[9]))->default_value(true),"Edge triggering for channel 10")
		("c11e",po::value<bool>(&(channeledge[10]))->default_value(true),"Edge triggering for channel 11")
		("c12e",po::value<bool>(&(channeledge[11]))->default_value(true),"Edge triggering for channel 12")
		("c13e",po::value<bool>(&(channeledge[12]))->default_value(true),"Edge triggering for channel 13")
		("c14e",po::value<bool>(&(channeledge[13]))->default_value(true),"Edge triggering for channel 14")
		("c15e",po::value<bool>(&(channeledge[14]))->default_value(true),"Edge triggering for channel 15")
		("c16e",po::value<bool>(&(channeledge[15]))->default_value(true),"Edge triggering for channel 16")
	;

	po::options_description cmdline;
	cmdline.add(genopt).add(ttopt).add(copt);

	po::options_description configfile;
	configfile.add(ttopt).add(copt);

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

	cout << "UQD-LOGIC-16 tag streamer version " << UQD_VERSION << endl << endl;

	//Make sure all settings are within bounds
	ASSERT(buffersize >= 5000000,-5);
	WARN(buffersize < 100000000,,"The buffer size chosen is a bit small. Will try to continue.\n");
	ASSERT(ledbrightness <=100 && ledbrightness >=0,-6);

	for (int i=0;i<16;i++) {
		//According the documentation, the range of voltage is -1.5V to 1.5V
		WARN(channelvoltage[i] > 1.5,channelvoltage[i] = 1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",channelvoltage[i],i+1,1.5);
		WARN(channelvoltage[i] < -1.5,channelvoltage[i] = -1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",channelvoltage[i],i+1,-1.5);
	}

	//Print out settings
	cout << "Buffer Size:         " << buffersize << endl;
	cout << "Use 10MHz Reference: " << (reference?"true":"false") << endl;
	cout << "AutoStart:           " << (autostart?"true":"false") << endl;
	cout << "Device ID:           " << deviceID << endl;

	cout << endl;

	cout << "Channel Settings:" << endl;
	for (int i=0;i<16;i++) {
		cout << i+1 << ": " << channelvoltage[i] << "V (" << (channeledge[i]?"+":"-") << ")\n";
	}
	cout << endl;


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
	try {
		try {
			tagger.Open(deviceID);
		
			ASSERT(tagger.IsOpen(),-1);

			cout << "> FPGA: " << tagger.GetFpgaVersion() << " Channels: " << tagger.GetNoInputs() << " Resolution: " << tagger.GetResolution() << endl;

			cout << "> Setting LED brightness" << endl;
			tagger.SetLedBrightness(ledbrightness);

			cout << "> Setting Channel Voltages" << endl;
			for (int i=0;i<16;i++) {
				tagger.SetInputThreshold(i+1,channelvoltage[i]);
			}
			cout << "> Setting Reference value" << endl;
			tagger.Use10MHz(reference);

			cout << "> Setting Edge Values" << endl;
			int mask = 0;
			for (int i=0;i<16;i++) {
				if (!channeledge[i]) mask+= 1<<i;
			}
			tagger.SetInversionMask(mask);

		} catch (TimeTag::Exception ex)
		{
			printf ("\nErr: %s\n", ex.GetMessageText().c_str());
			system("pause");
			return -1;
		}

		cout << "> Creating Buffer" << endl;

		//Create the time tagger's buffer
		int freebuffer = tt_getNextFree();
		buffer = tt_create(freebuffer,buffersize);
		if (!buffer) {
			cout << "ERROR: Couldn't create buffer" << endl;
			tagger.Close();
			return -2;
		}

		tt_setchannels(buffer,tagger.GetNoInputs());
		tt_setresolution(buffer,tagger.GetResolution());
		cout << "> Using buffer " << freebuffer << endl;

		//If it is to start automatically, add a runner
		if (autostart) {
			tt_addrunner(buffer);
		}

		uint64_t tagcount = 0;
		int err;
		int overflownum;
		uint64_t tag_offset = 0;
		uint64_t last = 0;
		while (runAcquisition) {
			while (!tt_running(buffer)) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(200));
			}
			printf("> Started Acquisition                              \n");

			//The time tagger seems to reset the clock between runs.

			tagger.StartTimetags();
			while (tt_running(buffer) && runAcquisition) {
				err = tagger.ReadErrorFlags();
				if (err) {
					cout << ">> ERR: " << tagger.GetErrorText(err);
				}

				tagcount = tagger.ReadTags(channelarray,tagarray);
				tt_writeindex(buffer)+=tagcount;
				for (int i=0; i<tagcount;i++) {
					if (last > tagarray[i]+tag_offset) {
						tag_offset = 100000000+ last - tagarray[i];
						cout << ">> WARNING: Offset tag detected." << endl;
					}
					if (channelarray[i]==30) {
						printf(">> OVERFLOW %i                          \n",overflownum++);
					} else {
						tt_tag(buffer,tt_datanum(buffer))  = tagarray[i]+tag_offset;
						tt_channel(buffer,tt_datanum(buffer)) = channelarray[i] -1;
						tt_datanum(buffer)++;
					}
					last = tagarray[i]+tag_offset;
				}
				tt_writeindex(buffer) = tt_datanum(buffer);
				printf(" Reading: %llu                       \r",tt_datapoints(buffer));
			}
			tagger.StopTimetags();
			printf("> Stopped Acquisition                              \n");
		}

		printf("> Cleaning Up...                    \n" );

		tt_close(buffer);
		tagger.Close();
	} catch (...) {
		cout << "ERROR: An unknown exception was caught!";
		system("pause");
	}

	
	return 0;
}
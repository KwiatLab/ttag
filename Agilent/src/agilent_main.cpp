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

#include <cstdlib>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
	using namespace std;

#include <boost/thread.hpp>
#include <boost/program_options.hpp>
	namespace po = boost::program_options;

#include "tt_agilent.h"	//Includes TC890 simplifying library
#include "ttag.h"		//Includes the timetagger buffer


#define ATT_VERSION 0.1f	//The program's version

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

void streamTagsToBuffer(tt_buf* buffer, ViInt32* mbuffer, uint64_t mbsize, uint64_t volatile* writeloc) {
	uint64_t readloc = 0;
	uint64_t i;
	ViInt32* mb;	//A simplifying array for mbuffer
	uint8_t channel;
	uint64_t count;
	uint64_t offset = 0;	//The offset keeps track of the wraparounds of the time tagger clock

	while (runAcquisition) {
		while (readloc < *writeloc && runAcquisition) {
			mb = &(mbuffer[(readloc % mbsize)*TC890_DMASIZE]);

			//Prepare the array for write
			tt_writeindex(buffer)+=TC890_DMASIZE;

			for (i=0; i < TC890_DMASIZE; i++) {
				if (mb[i]) {

					channel = (mb[i] & 0x70000000) >> 28;
					count = (mb[i] & 0x0fffffff) + offset;

					if (channel > 0 && channel <=6) {

						if ((mb[i] & 0x0fffffff) >= 209715200) {
							printf("ERROR: Tag out of bounds! (Check firmware version!)\n\tChannel: %i\n\tTag: %i\n\tTagValue: 0x%08x\n",(int)channel,(int)count,(int)i,mb[i]);
							runAcquisition = 0;
							break;
						}

						//Fix wrap-around
						if (count < tt_reference(buffer)) {

							//The Agilent has an out-of-order tag problem - check if this count is out of order
							if (count > offset+209715200/2) {
								//Aww shit. Now we have to go into damage-control mode - go back through the tags, and insert this one in the correct place
								//NOTE: This is not very nice to other apps using this at the same time. In fact, if tt_correlate runs on it while it replaces counts, things could
								//	get unstable!
								uint64_t backindex = tt_datanum(buffer)-1;

								//BTW: Reference not necessarily most recent tag, if user messes with it. Well, let the user feel the pain of crashing if he/she dares!
								while (backindex >0 && tt_tag(buffer,backindex) > count) {
									tt_tag(buffer,backindex+1) = tt_tag(buffer,backindex);
									tt_channel(buffer,backindex+1) = tt_channel(buffer,backindex);
									backindex--;
								}

								tt_tag(buffer,backindex+1) = count;
								tt_channel(buffer,backindex+1) = channel-1;

								//printf("Fixed wrap-error @%llu           \n",tt_datanum(buffer));

								goto addpoint;	//Hell yeah, a f**king goto.
							} else {
								//There was a wraparound - add it to offset and the tag
								offset+= 209715200;
								count += 209715200;
							}
						}

						if (count - tt_reference(buffer) > 200000000) {
							//printf("Noticed out-of-order error @%llu           \n",tt_datanum(buffer));
							count -= 209715200;	//Go back with the tag

							//Aww shit. Now we have to go into damage-control mode - go back through the tags, and insert this one in the correct place
							//NOTE: This is not very nice to other apps using this at the same time. In fact, if tt_correlate runs on it while it replaces counts, things could
							//	get unstable!
							uint64_t backindex = tt_datanum(buffer)-1;

							//BTW: Reference not necessarily most recent tag, if user messes with it. Well, let the user feel the pain of crashing if he/she dares!
							while (backindex >0 && tt_tag(buffer,backindex) > count) {
								tt_tag(buffer,backindex+1) = tt_tag(buffer,backindex);
								tt_channel(buffer,backindex+1) = tt_channel(buffer,backindex);
								backindex--;
							}

							tt_tag(buffer,backindex+1) = count;
							tt_channel(buffer,backindex+1) = channel-1;

							goto addpoint;

						}

						//Check for fishy business
						if (count - tt_reference(buffer) > 209715200/3) {
							printf("Noticed something fishy at @%llu           \n",tt_datanum(buffer));
						}

						//Add the point to the buffer
						tt_channel(buffer,tt_datanum(buffer))=channel-1;
						tt_tag(buffer,tt_datanum(buffer))=count;

						addpoint:
						tt_datanum(buffer)++;
					}
				} else {break;}	//A 0 tag represents the end of this set of data
			}

			//Put the write index back to the number actually written
			tt_writeindex(buffer) = tt_datanum(buffer);

			readloc++;
			if (*writeloc-readloc >= mbsize) {
				printf("Error: Microbuffer overflow!\n");
			}
			printf(" Reading: %llu of %llu (-%llu)       \r",readloc,*writeloc,*writeloc-readloc);
			
		}
		//Don't waste CPU cycles if not running
		if (!tt_running(buffer)) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
}

void streamTagsToBuffer_DEBUG(tt_buf* buffer, ViInt32* mbuffer, uint64_t mbsize, uint64_t volatile* writeloc) {
	uint64_t readloc = 0;
	uint64_t i;
	ViInt32* mb;	//A simplifying array for mbuffer
	uint8_t channel;
	uint64_t count;

	while (runAcquisition) {
		while (readloc < *writeloc && runAcquisition) {
			mb = &(mbuffer[(readloc % mbsize)*TC890_DMASIZE]);

			tt_writeindex(buffer)+=TC890_DMASIZE;

			for (i=0; i < TC890_DMASIZE; i++) {
				if (mb[i]) {

					channel = (mb[i] & 0x70000000) >> 28;
					count = (mb[i] & 0x0fffffff);

					if (channel > 0 && channel <=6) {

						if (count >= 209715200) {
							printf("ERROR: Tag out of bounds! (Check firmware version!)\n\tChannel: %i\n\tTag: %i\n\tTagValue: 0x%08x\n",(int)channel,(int)count,(int)i,mb[i]);
						}

						//Fix wrap-around
						if (count < tt_reference(buffer)) {
							//The Agilent has an out-of-order tag problem - check if this count is out of order
							if (count > 209715200/2) {
								printf("Detected wrap-error @%llu           \n",tt_datanum(buffer));
							} 
						}

						//Add the point to the buffer
						tt_channel(buffer,tt_datanum(buffer))=channel-1;
						tt_tag(buffer,tt_datanum(buffer))=count;
						tt_datanum(buffer)++;
					}
				} else {break;}	//A 0 tag represents the end of this set of data
			}

			tt_writeindex(buffer) = tt_datanum(buffer);

			readloc++;
			if (*writeloc-readloc >= mbsize) {
				printf("Error: Microbuffer overflow!\n");
			}
			printf(" Reading: %llu of %llu (-%llu)       \r",readloc,*writeloc,*writeloc-readloc);
			
		}
		//Don't waste CPU cycles if not running
		if (!tt_running(buffer)) {
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
	}
}

/*
void streamTagsToBuffer_DEBUG(tt_buf* buffer, ViInt32* mbuffer, uint64_t mbsize, uint64_t volatile* writeloc) {
	uint64_t readloc = 0;
	uint64_t i;
	ViInt32* mb;	//A simplifying array for mbuffer
	uint8_t channel;
	uint64_t count;
	uint64_t offset = 0;	//The offset keeps track of the wraparounds of the time tagger clock
	
	bool wraplast = false;

	//Change channel number to 6 channels + 4 Debug channels (marker channel, time wraparound, common hit, buffer switch)
	tt_setchannels(buffer,6+6);

	while (runAcquisition) {
		while (readloc < *writeloc && runAcquisition) {
			mb = &(mbuffer[(readloc % mbsize)*TC890_DMASIZE]);
			for (i=0; i < TC890_DMASIZE; i++) {
				if (mb[i]) {

					channel = (mb[i] & 0x70000000) >> 28;
					count = (mb[i] & 0x0fffffff); //+ offset;

					if (channel > 0 && channel <=6) {

						if ((mb[i] & 0x0fffffff) >= 209715200) {
							printf("COUNT FAIL %i %i -- %i (0x%08x)\n",(int)channel,(int)count,(int)i,mb[i]);
							tt_channel(buffer,tt_datanum(buffer))=11;
							tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);
							tt_datanum(buffer)++;
						}

						//Fix wrap-around
						if (count < tt_reference(buffer)) {

							if (count > 209715200/2) {
								printf("Fixed wrap-error @%llu                   \n",tt_datanum(buffer));
							}
							if (wraplast==true) {
								printf("Werr: %i (%i,%i)\n",(int)(count+209715200-tt_reference(buffer)),(int)(readloc%mbsize),(int)i);

								//There was a wraparound - unwrap it.
								//The wraparound is when for some reason, the tags come out of order. 
								//I need to fix the problem,therefore, by changing the offset times for the most recent couple of data points

								//offset+= 209715200;
								//count += 209715200;

								//Detected a wraparound error
								tt_channel(buffer,tt_datanum(buffer))=10;
								tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);
								tt_datanum(buffer)++;

								
								
							} else{
							//There was a wraparound - add it to offset and the tag
							//offset+= 209715200;
							//count += 209715200;

							//Add a wraparound tag
							tt_channel(buffer,tt_datanum(buffer))=8;
							tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);//count;
							tt_datanum(buffer)++;

							wraplast= true;
							}
							
							
						} else {wraplast = false;}
							
						//Add the point to the buffer
						tt_channel(buffer,tt_datanum(buffer))=channel-1;
						tt_tag(buffer,tt_datanum(buffer))=count;
						tt_datanum(buffer)++;
						

						

					} else if (channel > 6) {
						//DEBUG: Add marker channel
						tt_channel(buffer,tt_datanum(buffer))=6;
						tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);
						tt_datanum(buffer)++;
					} else if (channel==0) {
						//DEBUG: Add COMMON channel
						tt_channel(buffer,tt_datanum(buffer))=7;
						tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);
						tt_datanum(buffer)++;
					}
				} else {break;}	//A 0 tag represents the end of this set of data
			}

			//Add a buffer flip tag
			tt_channel(buffer,tt_datanum(buffer))= 9;
			tt_tag(buffer,tt_datanum(buffer))=tt_reference(buffer);
			tt_datanum(buffer)++;

			readloc++;
			printf(" Reading: %llu of %llu (-%llu)       \r",readloc,*writeloc,*writeloc-readloc);
			
		}
	}
}
*/

int stopfnc(ViInt32* data, int datasize,void* buffer) {
	return !tt_running((tt_buf*)buffer);
}

void runTimeTagger(tt_buf* buffer, ViSession timeTaggerID, ViInt32* mbuf,uint64_t mbufsize,uint64_t volatile* writeloc) {
	printf("> Time tagger ready!                  \n");
	while (runAcquisition) {
		if (tt_running(buffer)) {
			printf("> Started Acquisition                              \n");
			att_runAcquisition(timeTaggerID,mbuf,mbufsize,writeloc,&runAcquisition,stopfnc,(void*)buffer);
			printf("> Stopped Acquisition                              \n");
		} else {
			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
		}
	}
}

//Upon catching a ctrl+c it exits
static void exitsignal(int signal) {
    printf( "> Stopping Agilent                         \n" );
    runAcquisition = 0; //Tell it to stop getting data
}

   
//    threadData* data;
//    int timeTaggerAmount;   //Number of timeTaggers detected

int main(int argc, char** argv) {
	//Internal variables
	boost::thread DMAthread;	//The thread which runs straight acquisition
	tt_buf* buffer;				//The libTTag buffer: tags are written to it
	ViInt32* mbuffer;			//The internal microbuffer. It is used as the location for DMA copy from time tagger
	ViSession timetaggerID;		//The time tagger's ID when opened.
	uint64_t volatile mwriteloc = 0;		//The location writer is at in the DMA buffer

	//Variables that can be set by the user through command line options or configuration file
	uint64_t buffersize;		//The size of libTTag's buffer
	uint64_t microbuffersize;	//The size of an initial DMA buffer (the buffer size is microbuffersize*DMASIZE)
	string timeTaggerSN;		//There can be multiple time taggers available. timeTaggerID allows to choose time tagger by ID (optional)
	bool reference;				//The time taggers have a 10MHz reference input. This boolean chooses whether or not to use it. true=use, false=use internal clock
	bool debugchannels;			//If true, additional channels are added to the data stream coming from the time tagger which notify of time wraparounds, common inputs, etc.
	string configfilename;		//Name of configuration file from which to load settings
	float channelvoltage[6];	//The threshold voltage for each channel
	bool channeledge[6];		//The edge to trigger each channel: true=rising, false=falling
	bool veto;					//Whether or not veto is enabled
	float vetovoltage;			//Threshold voltage for veto channel
	bool vetoedge;				//The edge to trigger veto
	bool autostart;				//If true, starts taking data immediately (adds a runner)
	int	agilentbuffer;			//Agilent internal buffer. Setting this to a smaller value allows data to stream despite a low rate of counts.
	
	//These are command-line only options
    po::options_description genopt("Generic Options");
	genopt.add_options()
		("help,h","Show usage and help message")
		("config,c",po::value<string>(&configfilename)->default_value("ttAgilent.agcfg"),"Set configuration file to use")
    ;
	po::positional_options_description pos;
	pos.add("config",1);

	//These are options available from both command line and configuration file
	po::options_description ttopt("Time Tagger Options");
	ttopt.add_options()
		("buffer,b",po::value<uint64_t>(&buffersize)->default_value(100000000),"Choose size of the time tag buffer")
		("reference,r",po::value<bool>(&reference)->default_value(false),"Whether or not to use the 10MHz reference input")
		("microbuffer,m",po::value<uint64_t>(&microbuffersize)->default_value(20),"Size of the internal streaming buffer, as a multiple of 'agilentbuffer'.")
		("debug,d",po::value<bool>(&debugchannels)->default_value(false),"If true, dumps raw tags without post-processing (FOR DEBUGGING TIME TAGGER ONLY).")
		("veto",po::value<bool>(&veto)->default_value(true),"Whether or not veto input is enabled.")
		("autostart",po::value<bool>(&autostart)->default_value(false),"If enabled, starts taking data immediately (adds runner)")

		("agilentbuffer",po::value<int>(&agilentbuffer)->default_value(TC890_MEMSIZE),"The size, in bytes of the Agilent's internal buffer, up to 8MB. Smaller values allows more frequent data updates.")
    ;

	po::options_description copt("Channel Options");
	copt.add_options()
		("c1v",po::value<float>(&(channelvoltage[0]))->default_value(1.0),"Threshold voltage for channel 1")
		("c2v",po::value<float>(&(channelvoltage[1]))->default_value(1.0),"Threshold voltage for channel 2")
		("c3v",po::value<float>(&(channelvoltage[2]))->default_value(1.0),"Threshold voltage for channel 3")
		("c4v",po::value<float>(&(channelvoltage[3]))->default_value(1.0),"Threshold voltage for channel 4")
		("c5v",po::value<float>(&(channelvoltage[4]))->default_value(1.0),"Threshold voltage for channel 5")
		("c6v",po::value<float>(&(channelvoltage[5]))->default_value(1.0),"Threshold voltage for channel 6")

		("c1e",po::value<bool>(&(channeledge[0]))->default_value(true),"Edge triggering for channel 1 (1=rising,0=falling)")
		("c2e",po::value<bool>(&(channeledge[1]))->default_value(true),"Edge triggering for channel 2")
		("c3e",po::value<bool>(&(channeledge[2]))->default_value(true),"Edge triggering for channel 3")
		("c4e",po::value<bool>(&(channeledge[3]))->default_value(true),"Edge triggering for channel 4")
		("c5e",po::value<bool>(&(channeledge[4]))->default_value(true),"Edge triggering for channel 5")
		("c6e",po::value<bool>(&(channeledge[5]))->default_value(true),"Edge triggering for channel 6")

		("vetov",po::value<float>(&vetovoltage)->default_value(1.0),"Threshold voltage for veto channel, if enabled")
		("vetoe",po::value<bool>(&vetoedge)->default_value(true),"Edge triggering for veto channel, if enabled")
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

	cout << "Agilent TC890 tag streamer version " << ATT_VERSION << endl << endl;

	//Make sure all settings are within bounds
	ASSERT(microbuffersize > 2,-5);
	ASSERT(buffersize >= 50000000,-5);
	WARN(agilentbuffer % 4 != 0 ,agilentbuffer += agilentbuffer %4,"Agilent buffer not a multiple of 4. Adapting value!");
	WARN(agilentbuffer > TC890_MEMSIZE,agilentbuffer = TC890_MEMSIZE,"Agilent buffer too large. Adapting to maximum accepted value (%i).",TC890_MEMSIZE);
	WARN(agilentbuffer < 4*1024,agilentbuffer =4*1024,"Agilent buffer too small. Adapting to minimum accepted value (4096).");
	WARN(buffersize < 100000000,,"The buffer size chosen is a bit small. Will try to continue.\n");

	for (int i=0;i<6;i++) {
		//According the documentation, the range of voltage is -1.5V to 1.5V
		WARN(channelvoltage[i] > 1.5,channelvoltage[i] = 1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",channelvoltage[i],i+1,1.5);
		WARN(channelvoltage[i] < -1.5,channelvoltage[i] = -1.5f,"Threshold of %.2fV exceeds channel %i's max. Rounding to %.2fV.",channelvoltage[i],i+1,-1.5);
	}

    WARN(vetovoltage > 1.5,vetovoltage = 1.5f,"Threshold of %.2fV exceeds the veto's max. Rounding to %.2fV.",vetovoltage,1.5);

    //Print out settings
	cout << "Buffer Size:         " << buffersize << endl;
	cout << "DMA Buffer:          " << microbuffersize << endl;
	cout << "Agilent Buffer:      " << agilentbuffer << endl;
	cout << "AutoStart:           " << (autostart?"true":"false") << endl;
	cout << "Use 10MHz Reference: " << (reference?"true":"false") << endl;
	cout << "Veto Enabled:        " ;
	if (veto) {
		cout << "true -> " << vetovoltage << "V (" << (vetoedge?"+)":"-)") <<endl;}
	else {
		cout << "false" <<endl;
	}
	cout << "Debug Mode:          " << (debugchannels?"true":"false") << endl;

	cout << endl;

	cout << "Channel Settings:" << endl;
	for (int i=0;i<6;i++) {
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

	//Find the time tagger that we are looking for
	int timeTaggerNumber = att_getTimeTaggerAmount();

	cout << "> Found " << timeTaggerNumber << " time tagger" << (timeTaggerNumber==1?"":"s") << endl;

	//Fail if no time taggers
    if (timeTaggerNumber<=0) {
        ERROR(-1,"No Time Tagger found! Exiting.");
    }

	cout << "> Initializing" << endl;

	//Initialize time tagger
	timetaggerID = att_initializeTimeTagger(0,0,reference,8.0,agilentbuffer);

	//Allocate the DMA microbuffer
	mbuffer = new ViInt32[microbuffersize*TC890_DMASIZE];

	//Create the time tagger's buffer
	int freebuffer = tt_getNextFree();
	buffer = tt_create(freebuffer,buffersize);

	//Make sure the time tagger was created, and that a buffer was created for it successfully
    if (timetaggerID == 0 || !buffer || !mbuffer) {
        //Oh no! The time tagger failed to initialize!
        fprintf(stderr,"ERROR: Failed Initialization!\n");
            
        //Makes sure we never enter the acquisition loop
        runAcquisition = 0;
    } else {

		//Set all channels to their required values
		for (int i=0;i<6;i++) {
			att_setChannel(timetaggerID,i+1,channelvoltage[i],channeledge[i]);
		}

		//Set veto channel
		if (veto) {
			att_setChannel(timetaggerID,-2,vetovoltage,vetoedge);
		} else {
			att_disableChannel(timetaggerID,-2);
		}

		//Set the time tagger's resolution
		tt_setresolution(buffer,5e-11);
		//Set the number of channels
		tt_setchannels(buffer,6);

		//If it is to start automatically, add a runner
		if (autostart) {
			tt_addrunner(buffer);
		}

		cout << "> Using buffer " << freebuffer << endl;

		//Start acquisition of data
		DMAthread = boost::thread(boost::bind(&runTimeTagger,buffer,timetaggerID,mbuffer,microbuffersize,&mwriteloc));

		if (debugchannels)
			streamTagsToBuffer_DEBUG(buffer,mbuffer,microbuffersize,&mwriteloc);
		else
			streamTagsToBuffer(buffer,mbuffer,microbuffersize,&mwriteloc);

		DMAthread.join();
	}

	printf("> Cleaning Up...                    \n" );

	tt_close(buffer);
	delete[] mbuffer;
	//Acqrs_closeAll();	//I am not sure if this is relevant if eact time tagger is in its own process

	system("pause");
    return 0;
}

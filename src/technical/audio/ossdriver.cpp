/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ossdriver.h"

#include "src/foundation/logger.h"

#include <sys/ioctl.h>
#if defined(__linux__)
#include <linux/soundcard.h>
#endif
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <sys/soundcard.h>
#endif	
#include <fcntl.h>
#include <errno.h>


// Help function for starting a new thread
extern "C" void* startupThread(void *arg)
{
	OSSDriver *tmp = (OSSDriver*)arg;
	tmp->play();
	pthread_exit(0);
}


OSSDriver::OSSDriver(const char *device)
{
	assert(device != NULL);
	audioFD = -1;
	stopPlaying = 1;
	audioDevice = new char[strlen(device) + 1];
	strcpy(audioDevice, device);
}


OSSDriver::~OSSDriver()
{
	delete [] audioDevice;
	audioDevice = NULL;
	
	if (audioFD != -1) {
		close(audioFD);
	}
}


void OSSDriver::play()
{
	if (audioFD != -1 && audioFD != EBUSY) {
		AudioFormat *tmp = audioFiles.back();
		if ( tmp->open() != -1 ) {
			// How many bytes can be written to the buffer
			int avail = sizeof(audioBuffer);
			// Fill the buffer with up to 'avail' bytes
			int written = tmp->fillBuffer(audioBuffer, avail);
			
			// While the ``producer'' has written more than zero
			// bytes to the buffer
			while (written > 0 && stopPlaying == 0) {
				// Flush it to the device
				write(audioFD, audioBuffer, written);
				// and fill again
				written = tmp->fillBuffer(audioBuffer, avail);
			}
			tmp->close();
		}
	}
}


void OSSDriver::addAudioFile(AudioFormat *audioFile)
{
	audioFiles.push_back(audioFile);
}


void OSSDriver::playInThread()
{
	pthread_t audioThread;
	pthread_create(&audioThread, NULL, startupThread, this);
	audioThreads.push_back(audioThread);
}


bool OSSDriver::initialize()
{
	Logger::get().logDebug("Tries to initialize the sound device");
	// Opens the device write only and sets the flag to non-blocking.
	// This means that the call imidiately returns with a return value
	audioFD = open(audioDevice, O_WRONLY | O_NONBLOCK, 0);
	if (audioFD == -1 || audioFD == EBUSY) {
		fprintf(stderr, "Cannot open device: %s\n", strerror(errno));
		return false;
	}
	
	// Turns on blocking (disables the non-blocking flag)
	fcntl(audioFD, F_SETFL,0);
	
	// Storing the request and its argument
	int initValues[2][4] = 
	{ 
		{SNDCTL_DSP_RESET, SNDCTL_DSP_SETFMT, SOUND_PCM_WRITE_CHANNELS, SOUND_PCM_WRITE_RATE},
		{1, AFMT_S16_LE, 2, 44100} 
	};
	
	for (int i = 0; i < 4; i++) {
		// Initializes the audio device
		if ( !setIoctl(initValues[0][i], initValues[1][i]) ) {
			close(audioFD);
			audioFD = -1;
			return false;
		}
	}
	
	stopPlaying = 0;
	return true;
}


bool OSSDriver::setIoctl(int request, int param)
{
	int p = param;
	int ret = ioctl(audioFD, request, &p);
	if (ret == -1) {
		fprintf(stderr, "Initializing failed: %s\n", strerror(errno));
		return false;
	}
	else if (p != param) {
		printf("Device has not support for requested argument %d\n", param);
		return false;
	}
	return true;
}


void OSSDriver::shutdown()
{
	if (audioFD != -1) {
		Logger::get().logDebug("Shutdowns the audio device");
		
		stopPlaying = 1;
		
		// Wait for the threads to terminate
		unsigned int numElem = audioThreads.size();
		for (unsigned int i = 0; i < numElem; i++) {
			pthread_join(audioThreads[i], NULL);
		}
		audioThreads.clear();
		
		numElem = audioFiles.size();
		for (unsigned int i = 0; i < numElem; i++) {
			// Just to ensure that the file is closed
			audioFiles[i]->close();
		}
		audioFiles.clear();
		
		close(audioFD);
		audioFD = -1;
	}
}

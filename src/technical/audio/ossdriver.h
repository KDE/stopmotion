/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#ifndef OSSDRIVER_H
#define OSSDRIVER_H

#include "audiodriver.h"
#include <vector>
#include <pthread.h>
#include <unistd.h>

/**
 * OSS audio driver. This takes care of initializing the registered audio
 * device with differents parameters. The device will then by ready to
 * to play sound if the initializing was successfull.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class OSSDriver : public AudioDriver
{
public:
	/**
	 * Registers the given device.
	 * @param device the device to be used
	 */
	OSSDriver(const char *device);
	~OSSDriver();
	
	/**
	 * Function for playing PCM data. The registered audio format
	 * takes care of decoding to raw PCM. This function will not free the
	 * CPU until the playing is finished. Use the playInThread function if
	 * you want to play the sound in a separate thread.
	 */
	void play();
	
	/**
	 * Function for playing PCM data. It works excactly like the play
	 * function except that it plays in a separate thread.
	 */
	void playInThread();
	
	/**
	 * Function for adding a audio file which later on can be played
	 * with the play or playInThread functions.
	 * @param audioFile the audio file to be played
	 */
	void addAudioFile(AudioFormat *audioFile);
	
	/**
	 * Function for initializing the registered audio device.
	 * @return true on success, false otherwise
	 */
	bool initialize();
	
	/**
	 * Function for freeing the audio device so that other programs can use it.
	 */
	void shutdown();

private:
	/** Descriptor to the audio device. */
	int audioFD;
	
	/** Pointer to the registered audio device. */
	char *audioDevice;
	
	/** Buffer to be used on playing. This will be filled by the AudioFormat object. */
	char audioBuffer[4096];
	
	/** Contains all of the sounds to be played. */
	std::vector<AudioFormat*> audioFiles;
	
	/** Contains indentifications to the threads. */
	std::vector<pthread_t> audioThreads;
	
	/**
	 * Does a ioctl call with the given parameters. It checks if the call
	 * was successfully done.
	 * @param request the request
	 * @param param parameter to the request
	 * @return true on success, false otherwise
	 */
	bool setIoctl(int request, int param);
};

#endif

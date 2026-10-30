/*
 * SPDX-FileCopyrightText: 2005-2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include "audioformat.h"

/**
 * Interface to be used by the implemented sound drivers. They
 * will be responsible for initializing of the sound device and flushing 
 * PCM data to this device.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class AudioDriver 
{
public:
	virtual ~AudioDriver() {};
	
	/**
	 * Abstract function for playing PCM data. The registered audio format
	 * takes care of decoding to raw PCM. This function will not free the
	 * CPU until the playing is finished. Use the playInThread function if
	 * you want to play the sound in a separate thread.
	 */
	virtual void play() = 0;
	
	/**
	 * Abstract function for playing PCM data. It works exactly like the play
	 * function except that it plays in a separate thread.
	 */
	virtual void playInThread() = 0;
	
	/**
	 * Abstract function for adding a audio file which later on can be played
	 * with the play or playInThread functions. Ownership is not passed.
	 * @param audioFile the audio file to be played
	 */
	virtual void addAudioFile(AudioFormat *audioFile) = 0;
	
	/**
	 * Abstract function for initializing the registered audio device.
	 * @return true on success, false otherwise
	 */
	virtual bool initialize() = 0;
	
	/**
	 * Abstract function for freeing the audio device so that other programs
	 * can use it.
	 */
	virtual void shutdown() = 0;
};

#endif

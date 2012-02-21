/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
	 * Abstract function for playing PCM data. It works excactly like the play
	 * function except that it plays in a separate thread.
	 */
	virtual void playInThread() = 0;
	
	/**
	 * Abstract function for adding a audio file which later on can be played
	 * with the play or playInThread functions.
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

/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

class AudioFormat;

/**
 * Interface to be used by the implemented sound drivers. They
 * will be responsible for initializing of the sound device and flushing
 * PCM data to this device.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class AudioDriver {
public:
	virtual ~AudioDriver() = 0;

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
	 * @param audioFile the audio file to be played. Ownership is not passed.
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

/**
 * A simpler, safer interface to write a driver against.
 */
class SimpleAudioDriver {
public:
	/**
	 * Returns a @c SimpleAudioDriver wrapped in an AudioDriver.
	 * @param simpleDriver The driver to wrap. Ownership is passed.
	 * @return The wrapped driver. Ownership is returned.
	 */
	static AudioDriver* makeAudioDriver(SimpleAudioDriver* simpleDriver);
	virtual ~SimpleAudioDriver() = 0;
	/**
	 * Plays PCM data.
	 * @param buffer The buffer of PCM data. Ownership is not passed.
	 * @param bytes The number of bytes in the bufer.
	 */
	virtual void play(const char* buffer, int bytes) = 0;
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

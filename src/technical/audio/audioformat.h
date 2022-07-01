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
#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H

#include <stdint.h>

/**
 * Interface to be used by the implemented audio formats. They
 * will be responsible for decoding from their own format to raw PCM.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class AudioFormat {
public:
	virtual ~AudioFormat();

	/**
	 * Abstract function for opening the file registered with setFilename.
	 * @return 0 on success, -1 on failure
	 */
	virtual int open() = 0;

	/**
	 * Abstract function for closing the file registered with setFilename.
	 * @return 0 on success, -1 on failure
	 */
	virtual int close() = 0;

	/**
	 * Abstract function for resetting to the beginning of the sound.
	 */
	virtual void reset() = 0;

	/**
	 * Abstract function for filling the buffer with raw PCM data. It
	 * fills the buffer with up to 'numBytes' bytes.
	 * @param audioBuffer the buffer to be filled
	 * @param numBytes number of bytes available in the buffer
	 * @return number of bytes written to buffer
	 */
	virtual int fillBuffer(char *audioBuffer, int numBytes) = 0;

	/**
	 * Adds PCM data to the buffer.
	 * @param audioBuffer The start of the buffer to be added to
	 * @param count The number of values to be added to
	 * @return Number of values actually written. This must be
	 * no more than count and non-negative, and can only be 0
	 * if the sound is exhausted.
	 */
	virtual int add16bit(int16_t* audioBuffer, int count) = 0;

	/**
	 * Abstract function for retrieving the sound path.
	 * @return the sound path
	 */
	virtual const char* getSoundPath() const = 0;

	/**
	 * Retrieves the basename of the sound file.
	 * @return The filename with extension and without any path.
	 */
	virtual const char* getBasename() const = 0;
};

#endif

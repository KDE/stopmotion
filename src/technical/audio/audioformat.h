/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H

#include <cstdint>

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

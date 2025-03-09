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
#ifndef OGGVORBIS_H
#define OGGVORBIS_H

#include <stdint.h>

#include <vorbis/vorbisfile.h>

#include "audioformat.h"
#include "src/domain/animation/workspacefile.h"

class ErrorHandler;

/**
 * Class for decoding of oggvorbis data to raw PCM data.
 *
 * @author Bjoern Erik Nilsen & Fredrik B. Kjoelstad
 */
class OggVorbis : public AudioFormat {
public:
	OggVorbis();
	~OggVorbis() override;

	/**
	 * Function for registering the given filename to  be an ogg file. This
	 * function checks that the file can be opened and that it is a valid
	 * ogg file.
	 * @param file the filename to register
	 */
	void setFilename(WorkspaceFile& file, ErrorHandler& e);

	int open() override;
	int close() override;
	void reset() override;
	int fillBuffer(char *audioBuffer, int numBytes) override;
	int add16bit(int16_t* audioBuffer, int count) override;
	const char* getSoundPath() const override;
	const char* getBasename() const override;

private:
	/** The ogg representation of the file registered in this class. */
	OggVorbis_File *oggFile;

	/** The filename registered in this class. Hopefully a valid ogg file. */
	WorkspaceFile filename;
	enum {
		littleEndian = 0,
		bigEndian = 1,
		wordsAreUnsigned = 0,
		wordsAreSigned = 1,
		wordsAre8Bit = 1,
		wordsAre16Bit = 2,
		wordsAre32Bit = 4
	};
};

#endif

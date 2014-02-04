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
#include "oggvorbis.h"

#include "src/foundation/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


OggVorbis::OggVorbis() {
	oggFile  = NULL;
}


OggVorbis::~OggVorbis() {
	if (oggFile != NULL) {
		ov_clear(oggFile);
		free(oggFile);
		oggFile = NULL;
	}
}


void OggVorbis::setFilename(WorkspaceFile& file) {
	assert(file.path() != NULL);

	// Opens the file and tests for vorbis-ness
	FILE *f = fopen(file.path(), "r");
	if (f) {
		if (oggFile != NULL) {
			free(oggFile);
			oggFile = NULL;
		}
		
		oggFile = (OggVorbis_File*)malloc( sizeof(OggVorbis_File) );
		if (oggFile == NULL) {
			// logFatal terminates the application
			Logger::get().logFatal("Cannot allocate, out of memory!");
		}
		
		if ( ov_test(f, oggFile, NULL, 0) < 0 ) {
			Logger::get().logDebug("Not a valid oggfile");
			fclose(f);
			free(oggFile);
			oggFile = NULL;
			throw InvalidAudioFormatException();
		}
		
		// This also closes the file stream (f)
		ov_clear(oggFile);
		free(oggFile);
		oggFile = NULL;
		this->filename.swap(file);
	}
	else {
		Logger::get().logDebug("Cannot open file for reading");
		throw CouldNotOpenFileException();
	}
}


int OggVorbis::open()
{
	FILE *f = fopen(filename.path(), "r");
	if (f) {
		oggFile = (OggVorbis_File*)malloc( sizeof(OggVorbis_File) );
		if (oggFile == NULL) {
			// logFatal terminates the application
			Logger::get().logFatal("Cannot allocate, out of memory!");
		}
		if ( ov_open(f, oggFile, NULL, 0) == 0 ) {
			return 0;
		}
		fclose(f);
		free(oggFile);
		oggFile = NULL;
	}
	return -1;
}


int OggVorbis::close()
{
	if (oggFile) {
		ov_clear(oggFile);
		free(oggFile);
		oggFile = NULL;
		return 0;
	}
	return -1;
}


int OggVorbis::fillBuffer(char *audioBuffer, int numBytes)
{
	assert(audioBuffer != NULL);
	if (oggFile) {
		int crap;
		int ret = ov_read(oggFile, audioBuffer, numBytes, 0, 2, 1, &crap);
		return ret;
	}
	return 0;
}


const char* OggVorbis::getSoundPath() const {
	return filename.path();
}

const char* OggVorbis::getBasename() const {
	return filename.basename();
}


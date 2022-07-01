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
#include "../technical/audio/oggvorbis.h"

#include <stddef.h>
#include <stdint.h>

#include "src/domain/animation/workspacefile.h"

class ErrorHandler;

// A mock OggVorbis implementation that does nothing.

OggVorbis::OggVorbis() {
	oggFile = NULL;
}


OggVorbis::~OggVorbis() {
	close();
}


void OggVorbis::setFilename(WorkspaceFile& file, ErrorHandler&) {
	filename = file;
}


int OggVorbis::open() {
	return 0;
}


void OggVorbis::reset() {
}


int OggVorbis::close() {
	return 0;
}


int OggVorbis::fillBuffer(char*, int numBytes) {
	return numBytes;
}


int OggVorbis::add16bit(int16_t*, int count) {
	return count;
}


const char* OggVorbis::getSoundPath() const {
	return filename.path();
}

const char* OggVorbis::getBasename() const {
	return filename.basename();
}


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

#include "audioformat.h"

#include <vorbis/vorbisfile.h>
#include "workspacefile.h"

/**
 * Class for decoding of oggvorbis data to raw PCM data.
 *
 * @author Bjoern Erik Nilsen & Fredrik B. Kjoelstad
 */
class OggVorbis : public AudioFormat
{
public:
	
	/**
	 * Initializes the member pointers to NULL.
	 */
	OggVorbis();
	~OggVorbis();
	
	/**
	 * Function for registering the given filename to  be an ogg file. This
	 * function checks that the file can be opened and that it is a valid
	 * ogg file.
	 * @param filename the filename to register
	 */
	void setFilename(TemporaryWorkspaceFile& filename);
	
	/**
	 * Function for opening the file registered with setFilename.
	 * @return 0 on success, -1 on failure
	 */
	int open();
	
	/**
	 * Function for closing the file registered with setFilename.
	 * @return 0 on success, -1 on failure
	 */
	int close();
	
	/**
	 * Function to fill the buffer with PCM data. It fills the
	 * buffer with up to 'numBytes' bytes.
	 * @param audioBuffer the buffer to be filled
	 * @param numBytes number of bytes available in the buffer
	 * @return number of bytes written to buffer
	 */
	int fillBuffer(char *audioBuffer, int numBytes);
	
	/**
	 * Function for retrieving the sound path.
	 * @return the sound path
	 */
	char* getSoundPath();
	
private:
	/** The ogg representation of the file registered in this class. */
	OggVorbis_File *oggFile;
	
	/** The filename registred in this class. Hopefully a valid ogg file. */
	WorkspaceFile filename;
};

#endif

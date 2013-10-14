/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef SOUND_H_
#define SOUND_H_

#include "workspacefile.h"

#include <string>

class AudioFormat;
class TemporaryWorkspaceFile;

class Sound {
	AudioFormat* af;
	const char* name;
public:
	Sound();
	~Sound();
	/**
	 * Opens an audio file. See {@ref AudioFormat::setFilename} for
	 * exceptions that might be thrown.
	 * @param filename The filename to open. Ownership is not passed.
	 * @todo We need a way of mocking this for testing.
	 */
	void open(TemporaryWorkspaceFile& filename);
	/**
	 * Sets or resets the (human-readable) name of this sound.
	 * @param name The new name or NULL for no name. Ownership is passed.
	 * @return The old name or NULL for no name. Ownership is returned.
	 */
	const char* setName(const char* name);
	/**
	 * Sets the (human-readable) name of this sound. May only be used when
	 * there is no name already set for the sound.
	 * @param n The name to set.
	 */
	void setName(std::string& n);
	AudioFormat* getAudio();
	const AudioFormat* getAudio() const;
	const char* getName() const;
};

#endif

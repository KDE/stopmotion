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
#include "frame.h"

#include "src/technical/audio/audiodriver.h"
#include "src/domain/filenamevisitor.h"
#include "src/domain/animation/errorhandler.h"
#include "src/domain/animation/workspacefile.h"
#include "src/foundation/logger.h"
#include "src/config.h"
#include "sound.h"

#include <assert.h>
#include <string.h>
#include <memory>
#include <exception>
#include <sstream>
#include <string>


class SoundOutOfRangeException : public std::exception {
public:
	SoundOutOfRangeException() {
	}
    const char* what() const throw() {
    	return "Internal error: Sound out of range!";
    }
};


Frame::Frame(WorkspaceFile& file) {
	assert(file.path() != NULL);
	imagePath.swap(file);
}


Frame::~Frame() {
	int numElem = sounds.size();
	for (int i = 0; i < numElem; ++i) {
		delete sounds[i];
		sounds[i] = NULL;
	}
}


const char* Frame::getImagePath() const {
	assert(imagePath.path() != NULL);
	return imagePath.path();
}


const char* Frame::getBasename() const {
	assert(imagePath.basename() != 0);
	return imagePath.basename();
}

int Frame::newSound(WorkspaceFile& file) {
	Logger::get().logDebug("Adding sound in frame");
	preallocateSounds(1);
	std::unique_ptr<Sound> sound(new Sound());
	std::stringstream ss;
	std::stringstream::pos_type zeroOff = ss.tellp();
	ss << "Sound" << WorkspaceFile::getSoundNumber();
	int size = (ss.tellp() - zeroOff) + 1;
	char* soundName = new char[size];
	std::string cs = ss.str();
	strncpy(soundName, cs.c_str(), size);
	const char* oldName = sound->setName(soundName);
	assert(oldName == NULL);
	sound->open(file, *ErrorHandler::getThrower());
	WorkspaceFile::nextSoundNumber();
	sounds.push_back(sound.release());

	return 0;
}

void Frame::addSound(int index, Sound* sound) {
	if (index < 0 || soundCount() < index)
		throw SoundOutOfRangeException();
	sounds.insert(sounds.begin() + index, sound);
}

void Frame::preallocateSounds(int extra) {
	sounds.reserve(soundCount() + extra);
}

Sound* Frame::removeSound(int soundNumber) {
	if (soundNumber < 0 || soundCount() <= soundNumber)
		throw SoundOutOfRangeException();
	Sound* s = sounds[soundNumber];
	sounds.erase(sounds.begin() + soundNumber);
	return s;
}


Sound* Frame::getSound(int soundNumber) {
	if (soundNumber < 0 || soundCount() <= soundNumber)
		throw SoundOutOfRangeException();
	return sounds[soundNumber];
}


const Sound* Frame::getSound(int soundNumber) const {
	if (soundNumber < 0 || soundCount() <= soundNumber)
		throw SoundOutOfRangeException();
	return sounds[soundNumber];
}


int Frame::soundCount() const {
	return sounds.size();
}


const char* Frame::setSoundName(int soundNumber, const char* soundName) {
	if (soundNumber < 0 || soundCount() <= soundNumber)
		throw SoundOutOfRangeException();
	return sounds[soundNumber]->setName(soundName);
}


const char* Frame::getSoundName(int soundNumber) const {
	if (soundNumber < 0 || soundCount() <= soundNumber)
		throw SoundOutOfRangeException();
	return sounds[soundNumber]->getName();
}


void Frame::playSounds(AudioDriver *driver) const {
	SoundVector::const_iterator i = sounds.begin();
	for (; i != sounds.end(); ++i) {
		(*i)->addToDriver(*driver);
	}
	if (i != sounds.begin())
		driver->playInThread();
}

void Frame::replaceImage(WorkspaceFile& otherImage) {
	otherImage.swap(imagePath);
}

void Frame::accept(FileNameVisitor& v) const {
	v.visitImage(imagePath.path());
	for(SoundVector::const_iterator i = sounds.begin();
			i != sounds.end();
			++i) {
		v.visitSound((*i)->getSoundPath());
	}
}

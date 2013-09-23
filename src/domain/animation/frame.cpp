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
#include "frame.h"

#include "src/technical/audio/oggvorbis.h"
#include "src/technical/util.h"
#include "src/domain/filenamevisitor.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <libgen.h>
#include <memory>

Frame::Sound::Sound() : af(0), name(0) {
}

Frame::Sound::~Sound() {
	delete af;
	delete name;
}

/**
 *@todo check audio type (ogg, mp3, wav ...)
 */
void Frame::Sound::open(TemporaryWorkspaceFile& filename) {
	std::auto_ptr<AudioFormat> a(new OggVorbis());
	a->setFilename(filename);
	af = a.release();
}

const char* Frame::Sound::setName(const char* n) {
	const char* r = name;
	name = n;
	return r;
}

void Frame::Sound::setName(std::string& n) {
	assert(!name);
	int size = n.size();
	char* a = new char[size];
	name = a;
	strncpy(a, n.c_str(), size);
}

AudioFormat* Frame::Sound::getAudio() {
	return af;
}

const char* Frame::Sound::getName() const {
	return name;
}


Frame::Frame(TemporaryWorkspaceFile& filename) {
	assert(filename.path() != NULL);
	imagePath = filename;
}


Frame::~Frame() {
	unsigned int numElem = sounds.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete sounds[i];
		sounds[i] = NULL;
	}
}


const char* Frame::getImagePath() {
	assert(imagePath.path() != NULL);
	Logger::get().logDebug("Retrieving picture from frame: ");
	Logger::get().logDebug(imagePath.path());
	return imagePath.path();
}


int Frame::addSound(TemporaryWorkspaceFile& filename)
{
	Logger::get().logDebug("Adding sound in frame");
	preallocateSounds(1);
	std::auto_ptr<Sound> sound(new Sound());
	stringstream ss;
	ss << "Sound" << WorkspaceFile::getSoundNumber();
	unsigned int size = ss.tellp() + 1;
	char* soundName = new char[size];
	strncpy(soundName, ss.str().c_str(), size);
	char* oldName = sound->setName(soundName);
	assert(oldName == NULL);
	int ret = sound->open(filename);
	if (ret != 0) {
		return ret;
	}
	WorkspaceFile::nextSoundNumber();
	sounds.push_back(sound.release());

	return 0;
}

void Frame::addSound(Frame::Sound* sound, int index) {
	sounds.insert(sounds.begin() + index, sound);
}

void Frame::preallocateSounds(int extra) {
	sounds.reserve(sounds.size() + extra);
}

Frame::Sound* Frame::removeSound(int soundNumber)
{
	Frame::Sound* s = sounds[s];
	sounds.erase(sounds.begin() + soundNumber);
	return s;
}


unsigned int Frame::getNumberOfSounds( )
{
	return sounds.size();
}


const char* Frame::setSoundName(unsigned int soundNumber,
		const char* soundName) {
	return sounds[soundNumber]->setName(soundName);
}


const char* Frame::getSoundName(unsigned int soundNumber) {
	return sounds[soundNumber]->getName();
}


void Frame::playSounds(AudioDriver *driver) {
	SoundVector::iterator i = sounds.begin();
	for (; i != sounds.end(); ++i) {
		driver->addAudioFile((*i)->getAudio());
	}
	if (i != sounds.begin())
		driver->playInThread();
}


void Frame::accept(FileNameVisitor& v) const {
	v.visitImage(imagePath.path());
	for(SoundVector::const_iterator i = sounds.begin();
			i != sounds.end();
			++i) {
		v.visitSound((*i)->getAudio()->getSoundPath());
	}
}

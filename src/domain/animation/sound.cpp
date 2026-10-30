/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sound.h"

#include "src/technical/audio/audioformat.h"
#include "src/technical/audio/oggvorbis.h"
#include "src/technical/audio/audiodriver.h"

#include <cassert>
#include <cstring>
#include <memory>

Sound::Sound() : af(nullptr), name(nullptr) {
}

Sound::~Sound() {
	delete af;
	delete[] name;
}

/**
 *@todo check audio type (ogg, mp3, wav ...)
 */
void Sound::open(WorkspaceFile& file, ErrorHandler& e) {
	std::unique_ptr<OggVorbis> a(new OggVorbis());
	a->setFilename(file, e);
	delete af;
	af = a.release();
	af->open();
}

const char* Sound::setName(const char* n) {
	const char* r = name;
	name = n;
	return r;
}

void Sound::setName(std::string& n) {
	assert(!name);
	int size = n.size() + 1;
	char* a = new char[size];
	name = a;
	strncpy(a, n.c_str(), size);
}

const char* Sound::getName() const {
	return name;
}

const char* Sound::getSoundPath() const {
	return af->getSoundPath();
}

const char* Sound::getBasename() const {
	return af->getBasename();
}

void Sound::addToDriver(AudioDriver& ad) const {
	ad.addAudioFile(af);
}

/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sound.h"

#include "src/technical/audio/audioformat.h"
#include "src/technical/audio/oggvorbis.h"
#include "src/technical/audio/audiodriver.h"

#include <memory>

Sound::Sound() {
}

Sound::~Sound() {
}

/**
 *@todo check audio type (ogg, mp3, wav ...)
 */
void Sound::open(WorkspaceFile& file, ErrorHandler& e) {
	std::unique_ptr<OggVorbis> a(new OggVorbis());
	a->setFilename(file, e);
	af = std::move(a);
	af->open();
}

std::string Sound::setName(std::string n) {
	name.swap(n);
	return n;
}

const char* Sound::getName() const {
	return name.c_str();
}

const char* Sound::getSoundPath() const {
	return af->getSoundPath();
}

const char* Sound::getBasename() const {
	return af->getBasename();
}

void Sound::addToDriver(AudioDriver& ad) const {
	ad.addAudioFile(af.get());
}

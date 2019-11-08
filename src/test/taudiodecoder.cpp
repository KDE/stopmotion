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
#include "../technical/audio/qtdecoder.h"

// A mock audio decoder implementation that does nothing.

QtAudioDecoder::QtAudioDecoder(WorkspaceFile& f) {
	file.swap(f);
}

QtAudioDecoder::~QtAudioDecoder() {
	close();
}

int QtAudioDecoder::open() {
	return 0;
}

void QtAudioDecoder::reset() {
}

int QtAudioDecoder::close() {
	return 0;
}

int QtAudioDecoder::bytesAvailable() const {
	return 256;
}

int QtAudioDecoder::fillBuffer(char *, int numBytes) {
	return numBytes;
}

const char* QtAudioDecoder::getSoundPath() const {
	return file.path();
}

const char* QtAudioDecoder::getBasename() const {
	return file.basename();
}

void QtAudioDecoder::decodedData() {
}

void QtAudioDecoder::decoderStateChanged(QAudioDecoder::State) {
}

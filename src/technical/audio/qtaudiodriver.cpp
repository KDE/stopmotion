/***************************************************************************
 *   Copyright (C) 2019 by Linuxstopmotion contributors;                   *
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

#include "qtaudiodriver.h"
#include "src/foundation/logger.h"

#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QAudioOutput>

#include <memory>
#include <cstring>
#include <vector>

void QtAudioDriver::getAudioFormat(QAudioFormat& audioFormat) {
		audioFormat.setSampleRate(44100);
		audioFormat.setCodec("audio/x-raw");
		audioFormat.setChannelCount(2);
		audioFormat.setByteOrder(QAudioFormat::LittleEndian);
		audioFormat.setSampleType(QAudioFormat::UnSignedInt);
		audioFormat.setSampleSize(16);
}

class QtAudioDriver::Impl : public QIODevice {
	static const int buffer_size = 4096;
	// number of bytes required before we start playing a sound
	static const int buffer_ahead = 4096;
	typedef int16_t sample_t;
	QAudioFormat audioFormat;
	QAudioOutput* output;
	std::list<AudioFormat*> pendingSounds;
	std::list<AudioFormat*> sounds;
	std::vector<sample_t> buffer;
public:
	Impl() : output(0), buffer(buffer_size) {
		QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
		getAudioFormat(audioFormat);
		QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
		if (!info.isFormatSupported(audioFormat)) {
			// Trying to find out what Qt thinks it CAN do!...
			info = QAudioDeviceInfo::defaultOutputDevice();
			audioFormat = info.nearestFormat(audioFormat);
			// Is this OK?
		}
		output = new QAudioOutput(info, audioFormat);
		setOpenMode(ReadOnly);
	}
	~Impl() {
		delete output;
	}
	void addSound(AudioFormat* a) {
		a->reset();
		pendingSounds.push_back(a);
		Logger::get().logDebug("Added sound");
		output->stateChanged(QAudio::ActiveState);
	}
	void cueReadySounds() {
		std::list<AudioFormat*>::iterator s = sounds.begin();
		s = pendingSounds.begin();
		while (s != pendingSounds.end()) {
			qint64 available = (*s)->bytesAvailable();
			if (buffer_ahead <= available) {
				Logger::get().logDebug("new playable sound");
				// *s is now playable
				sounds.push_back(*s);
				s = pendingSounds.erase(s);
			} else {
				++s;
			}
		}
	}
	qint64 readChunk(char *data, quint64 maxlen) {
		// get the smallest number of bytes from all sounds playing
		qint64 bytesToGet = maxlen;
		bool anythingPlaying = false;
		std::list<AudioFormat*>::iterator s = sounds.begin();
		while (s != sounds.end()) {
			qint64 available = (*s)->bytesAvailable();
			if (0 < available) {
				anythingPlaying = true;
				if (available < bytesToGet) {
					bytesToGet = available;
				}
				++s;
			} else if ((*s)->isFinished()) {
				// no bytes avaiable and no more to come
				s = sounds.erase(s);
			} else {
				++s;
			}
		}
		if (!anythingPlaying) {
			Logger::get().logDebug("Nothing is playing now");
			std::fill(data, data + maxlen, '\0');
			return maxlen;
		}
		s = sounds.begin();
		int len = (*s)->fillBuffer(data, bytesToGet);
		if (len < bytesToGet) {
			Logger::get().logWarning("fillBuffer returned less than bytesAvailable %d < %d", len, (int)bytesToGet);
		}
		++s;
		for (; s != sounds.end(); ++s) {
			len = mixSound(*s, reinterpret_cast<sample_t*>(data), bytesToGet);
			if (len < bytesToGet) {
				Logger::get().logWarning("fillBuffer returned less than bytesAvailable");
			}
		}
		if (sounds.empty() && pendingSounds.empty()) {
			output->stateChanged(QAudio::IdleState);
			Logger::get().logDebug("out of sound");
		}
		Logger::get().logDebug("chunk: %d",bytesToGet);
		return bytesToGet;
	}
  qint64 readData(char *data, qint64 maxlen) {
		cueReadySounds();
		qint64 remaining = maxlen;
		while (0 < remaining) {
			if (sounds.empty()) {
				Logger::get().logDebug("Need to fill buffer with zeroes");
				std::fill(data, data + remaining, '\0');
				return maxlen;
			}
			qint64 chunkSize = readChunk(data, remaining);
			data += chunkSize;
			remaining -= chunkSize;
		}
		return maxlen;
	}
  qint64 writeData(const char *, qint64) {
		return 0;
	}
	void play() {
		output->start(this);
	}
	void stop() {
		output->stop();
	}
private:
	qint64 mixSound(AudioFormat* s, sample_t* out, qint64 bytes) {
		const qint64 maxbytes = buffer_size * sizeof(sample_t);
		char* buf = reinterpret_cast<char*>(&*buffer.begin());
		int len = s->fillBuffer(buf, std::min(bytes, maxbytes));
		int samples = len / sizeof(sample_t);
		for (int i = 0; i != samples; ++i) {
			out[i] += buf[i];
		}
		return len;
	}
};

QtAudioDriver::QtAudioDriver() : impl(0) {
	impl = new Impl();
}

QtAudioDriver::~QtAudioDriver() {
	delete impl;
}

void QtAudioDriver::play() {
}

void QtAudioDriver::playInThread() {
	impl->play();
}

void QtAudioDriver::addAudioFile(AudioFormat* audioFile) {
	impl->addSound(audioFile);
}

bool QtAudioDriver::initialize() {
	return true;
}

void QtAudioDriver::shutdown() {
	impl->stop();
}

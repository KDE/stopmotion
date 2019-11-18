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

template<typename T> struct audio_traits {};
template<> struct audio_traits<short> {
	static constexpr QAudioFormat::Endian endian = QAudioFormat::LittleEndian;
	static constexpr QAudioFormat::SampleType type = QAudioFormat::SignedInt;
	static constexpr int size = 16;
	static short add(short x, short y) { return x + y; }
};

template<> struct audio_traits<unsigned short> {
	static constexpr QAudioFormat::Endian endian = QAudioFormat::LittleEndian;
	static constexpr QAudioFormat::SampleType type = QAudioFormat::UnSignedInt;
	static constexpr int size = 16;
	static unsigned short add(unsigned short x, unsigned short y) { return x + y + 0x8000; }
};

template<> struct audio_traits<float> {
	static constexpr QAudioFormat::Endian endian = QAudioFormat::LittleEndian;
	static constexpr QAudioFormat::SampleType type = QAudioFormat::Float;
	static constexpr int size = 32;
	static float add(float x, float y) { return x + y; }
};

namespace {
// float also seems to work
typedef int16_t sample_t;
}

class QtAudioDriver::Impl : public QIODevice {
	static const int buffer_size = 4096;
	// number of bytes required before we start playing a sound
	static const int buffer_ahead = 4096;
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
		(*s)->fillBuffer(data, bytesToGet);
		++s;
		for (; s != sounds.end(); ++s) {
			mixSound(*s, reinterpret_cast<sample_t*>(data), bytesToGet);
		}
		if (sounds.empty() && pendingSounds.empty()) {
			output->stateChanged(QAudio::IdleState);
		}
		return bytesToGet;
	}
  qint64 readData(char *data, qint64 maxlen) {
		cueReadySounds();
		qint64 remaining = maxlen;
		while (0 < remaining) {
			if (sounds.empty()) {
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
			out[i] = audio_traits<sample_t>::add(out[i], buf[i]);
		}
		return len;
	}
};

void QtAudioDriver::getAudioFormat(QAudioFormat& audioFormat) {
		audioFormat.setSampleRate(44100);
		audioFormat.setCodec("audio/x-raw");
		audioFormat.setChannelCount(2);
		audioFormat.setByteOrder(audio_traits<sample_t>::endian);
		audioFormat.setSampleType(audio_traits<sample_t>::type);
		audioFormat.setSampleSize(audio_traits<sample_t>::size);
}

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

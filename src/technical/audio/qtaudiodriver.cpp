/***************************************************************************
 *   Copyright (C) 2017 by Linuxstopmotion contributors;                   *
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
#include "src/technical/audio/audioformat.h"

#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>
#include <QAudioOutput>
#include <QAudioSink>

#include <stdint.h>
#include <algorithm>
#include <list>
#include <vector>

class QtAudioDriver::Impl : public QIODevice {
	Q_OBJECT
	static const int buffer_size = 4096;
	typedef int16_t sample_t;
	QAudioFormat audioFormat;
	QAudioSink* output;
	std::list<AudioFormat*> sounds;
	std::vector<sample_t> buffer;
public:
    Impl() : output(0), buffer(buffer_size) {
		audioFormat.setSampleRate(44100);
		audioFormat.setChannelCount(2);
		audioFormat.setSampleFormat(QAudioFormat::Int16);
		QAudioDevice info=QMediaDevices::defaultAudioOutput();
		if (!info.isFormatSupported(audioFormat)) {
			audioFormat = info.preferredFormat();
		}
		output = new QAudioSink(info, audioFormat);
		setOpenMode(ReadOnly);
	}
	~Impl() {
		delete output;
	}
	void addSound(AudioFormat* a) {
		a->reset();
		sounds.push_back(a);
		Logger::get().logDebug("Added sound");
		emit output->stateChanged(QAudio::ActiveState);
	}
    qint64 readData(char *data, qint64 maxlen) {
		std::list<AudioFormat*>::iterator s = sounds.begin();
		int len = 0;
		for (; s != sounds.end(); ++s) {
			len = (*s)->fillBuffer(data, maxlen);
			if (len != 0)
				break;
			sounds.pop_front();
			s = sounds.begin();
		}
		while (s != sounds.end()) {
			if (mixSound(*s, reinterpret_cast<int16_t*>(data), len))
				++s;
			else
				s = sounds.erase(s);
		}
		if (len == 0) {
			emit output->stateChanged(QAudio::IdleState);
			Logger::get().logDebug("out of sound");
			return 0;
		}
		Logger::get().logDebug("Sent data %d", len);
		return len;
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
	// returns false if s is exhausted after the call
	bool mixSound(AudioFormat* s, sample_t* out, unsigned long length) {
		char* buf = reinterpret_cast<char*>(&*buffer.begin());
		while (0 < length) {
			int len2 = s->fillBuffer(buf,
				std::min(length, (unsigned long)buffer_size * sizeof(sample_t)));
			if (len2 == 0) {
				return false;
			}
			length -= len2;
			int samples = len2 / sizeof(sample_t);
			for (int i = 0; i != samples; ++i) {
				*out += buf[i];
				++out;
			}
		}
		return true;
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

#include "qtaudiodriver.moc"

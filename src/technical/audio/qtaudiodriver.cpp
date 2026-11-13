/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qtaudiodriver.h"
#include "stopmotion_debug.h"
#include "src/technical/audio/audioformat.h"

#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>
#include <QAudioOutput>
#include <QAudioSink>

#include <cstdint>
#include <algorithm>
#include <iterator>
#include <limits>
#include <list>
#include <vector>

class QtAudioDriver::Impl : public QIODevice {
	Q_OBJECT
	static const int buffer_size = 4096;
	using sample_t = int16_t;
	QAudioFormat audioFormat;
	std::unique_ptr<QAudioSink> output;
	std::list<AudioFormat*> sounds;
	std::vector<sample_t> buffer;
public:
    Impl() : buffer(buffer_size) {
		audioFormat.setSampleRate(44100);
		audioFormat.setChannelCount(2);
		audioFormat.setSampleFormat(QAudioFormat::Int16);
		QAudioDevice info=QMediaDevices::defaultAudioOutput();
		if (!info.isFormatSupported(audioFormat)) {
			audioFormat = info.preferredFormat();
		}
		output = std::make_unique<QAudioSink>(info, audioFormat);
		setOpenMode(ReadOnly);
	}
	~Impl() override {
	}
	void addSound(AudioFormat* a) {
		a->reset();
		sounds.push_back(a);
		qCDebug(STOPMOTION_LOG, "Added sound");
		Q_EMIT output->stateChanged(QAudio::ActiveState);
	}
    qint64 readData(char *data, qint64 maxlen) override {
		// The first sound with data left is copied, the others are mixed in.
		const int maxBytes = static_cast<int>(std::min<qint64>(maxlen,
				std::numeric_limits<int>::max()));
		int len = 0;
		while (!sounds.empty() && len == 0) {
			len = sounds.front()->fillBuffer(data, maxBytes);
			if (len == 0)
				sounds.pop_front();
		}
		if (len == 0) {
			Q_EMIT output->stateChanged(QAudio::IdleState);
			qCDebug(STOPMOTION_LOG, "out of sound");
			return 0;
		}
		auto s = std::next(sounds.begin());
		while (s != sounds.end()) {
			if (mixSound(*s, reinterpret_cast<sample_t*>(data), len))
				++s;
			else
				s = sounds.erase(s);
		}
		qCDebug(STOPMOTION_LOG, "Sent data %d", len);
		return len;
	}
    qint64 writeData(const char *, qint64) override {
		return 0;
	}
	void play() {
		output->start(this);
	}
	void stop() {
		output->stop();
	}
private:
	// Adds length bytes of sound s to out. Returns false if s is exhausted.
	bool mixSound(AudioFormat* s, sample_t* out, int length) {
		char* buf = reinterpret_cast<char*>(buffer.data());
		const int bufferBytes = buffer.size() * sizeof(sample_t);
		while (0 < length) {
			int len2 = s->fillBuffer(buf, std::min(length, bufferBytes));
			if (len2 == 0) {
				return false;
			}
			length -= len2;
			int samples = len2 / sizeof(sample_t);
			for (int i = 0; i != samples; ++i) {
				int sum = *out + buffer[i];
				*out = std::clamp(sum,
						int(std::numeric_limits<sample_t>::min()),
						int(std::numeric_limits<sample_t>::max()));
				++out;
			}
		}
		return true;
	}
};

QtAudioDriver::QtAudioDriver() : impl(std::make_unique<Impl>()) {
}

QtAudioDriver::~QtAudioDriver() {
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

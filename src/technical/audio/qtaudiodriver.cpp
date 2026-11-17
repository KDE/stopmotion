/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qtaudiodriver.h"
#include "stopmotion_debug.h"
#include "src/technical/audio/audioformat.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>
#include <QAudioSink>

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <limits>
#include <list>
#include <vector>

namespace {

using sample_t = int16_t;

/**
 * A sound being played, converted to the channels and sample rate of the
 * output. Samples keep the range of sample_t, as floats.
 */
class PlayingSound {
	std::shared_ptr<AudioFormat> sound;
	const int inChannels;
	const int outChannels;
	// source frames per output frame
	const double step;
	std::vector<sample_t> input;
	size_t inputBegin = 0;
	size_t inputEnd = 0;
	// output is interpolated between these two frames
	std::vector<float> current;
	std::vector<float> next;
	double position = 0;
	bool hasCurrent = false;
	bool hasNext = false;

	bool fill() {
		std::copy(input.begin() + inputBegin, input.begin() + inputEnd,
				input.begin());
		inputEnd -= inputBegin;
		inputBegin = 0;
		const int bytes = sound->fillBuffer(
				reinterpret_cast<char*>(input.data() + inputEnd),
				static_cast<int>((input.size() - inputEnd) * sizeof(sample_t)));
		inputEnd += bytes / sizeof(sample_t);
		return 0 < bytes;
	}
	bool fetch(std::vector<float>& frame) {
		while (inputEnd - inputBegin < size_t(inChannels)) {
			if (!fill())
				return false;
		}
		const sample_t* in = input.data() + inputBegin;
		inputBegin += inChannels;
		float left = in[0];
		float right = in[0];
		if (inChannels == 2 || inChannels == 4) {
			right = in[1];
		} else if (2 < inChannels) {
			// Vorbis order: front left, center, front right, ...
			const float center = in[1] * 0.7071f;
			left += center;
			right = in[2] + center;
		}
		if (outChannels == 1) {
			frame[0] = inChannels == 1? left : (left + right) / 2;
		} else {
			std::fill(frame.begin(), frame.end(), 0.0f);
			frame[0] = left;
			frame[1] = right;
		}
		return true;
	}
	void advance() {
		if (!hasNext) {
			hasCurrent = false;
			return;
		}
		current.swap(next);
		hasNext = fetch(next);
		if (!hasNext)
			next = current;
	}
public:
	PlayingSound(std::shared_ptr<AudioFormat> s, int channels, int rate)
		: sound(std::move(s)), inChannels(sound->channelCount()),
		  outChannels(channels),
		  step(0 < rate? double(sound->sampleRate()) / rate : 0),
		  input(4096), current(channels), next(channels) {
		if (0 < inChannels && 0 < step && 0 < outChannels) {
			input.resize(std::max<size_t>(input.size(), inChannels));
			hasCurrent = fetch(current);
			hasNext = hasCurrent && fetch(next);
			if (!hasNext)
				next = current;
		}
	}
	const AudioFormat* get() const {
		return sound.get();
	}
	/** Adds up to frameCount frames to out; returns how many were added. */
	int addTo(float* out, int frameCount) {
		int n = 0;
		for (; n != frameCount && hasCurrent; ++n) {
			const float f = static_cast<float>(position);
			for (int c = 0; c != outChannels; ++c)
				*out++ += current[c] + (next[c] - current[c]) * f;
			position += step;
			while (1.0 <= position && hasCurrent) {
				position -= 1.0;
				advance();
			}
		}
		return n;
	}
};

template<typename T> void store(char* out, T value) {
	std::memcpy(out, &value, sizeof(T));
}

QAudioFormat chooseFormat(const QAudioDevice& device) {
	QAudioFormat format;
	format.setSampleRate(44100);
	format.setChannelCount(2);
	format.setSampleFormat(QAudioFormat::Int16);
	if (!device.isNull() && !device.isFormatSupported(format)) {
		const QAudioFormat preferred = device.preferredFormat();
		if (preferred.isValid())
			format = preferred;
	}
	return format;
}

}

class QtAudioDriver::Impl : public QIODevice {
	Q_OBJECT
	static const int maxFrames = 16384;
	QAudioFormat audioFormat;
	std::unique_ptr<QAudioSink> output;
	std::list<PlayingSound> sounds;
	std::vector<float> mix;
	void convert(const float* in, int count, char* out) const {
		const int bytes = audioFormat.bytesPerSample();
		for (int i = 0; i != count; ++i, out += bytes) {
			const float v = in[i];
			switch (audioFormat.sampleFormat()) {
			case QAudioFormat::UInt8:
				store(out, static_cast<uint8_t>(
						std::clamp(v / 256 + 128, 0.0f, 255.0f)));
				break;
			case QAudioFormat::Int16:
				store(out, static_cast<int16_t>(
						std::clamp(v, -32768.0f, 32767.0f)));
				break;
			case QAudioFormat::Int32:
				store(out, static_cast<int32_t>(
						std::clamp(double(v) * 65536, -2147483648.0, 2147483647.0)));
				break;
			case QAudioFormat::Float:
				store(out, std::clamp(v / 32768, -1.0f, 1.0f));
				break;
			default:
				std::memset(out, 0, bytes);
				break;
			}
		}
	}
public:
	Impl(const QAudioDevice& device, const QAudioFormat& format)
		: audioFormat(format),
		  output(std::make_unique<QAudioSink>(device, format)) {
		setOpenMode(ReadOnly);
	}
	~Impl() override {
	}
	void addSound(std::shared_ptr<AudioFormat> a) {
		// playing a sound again restarts it
		sounds.remove_if([&a](const PlayingSound& p) { return p.get() == a.get(); });
		a->reset();
		sounds.emplace_back(std::move(a), audioFormat.channelCount(),
				audioFormat.sampleRate());
		qCDebug(STOPMOTION_LOG, "Added sound");
		Q_EMIT output->stateChanged(QAudio::ActiveState);
	}
	qint64 readData(char *data, qint64 maxlen) override {
		const int bytesPerFrame = audioFormat.bytesPerFrame();
		const int channels = audioFormat.channelCount();
		if (bytesPerFrame <= 0)
			return 0;
		const int frames = static_cast<int>(std::min<qint64>(
				maxlen / bytesPerFrame, maxFrames));
		mix.assign(size_t(frames) * channels, 0.0f);
		int produced = 0;
		for (auto s = sounds.begin(); s != sounds.end(); ) {
			const int n = s->addTo(mix.data(), frames);
			produced = std::max(produced, n);
			if (n < frames)
				s = sounds.erase(s);
			else
				++s;
		}
		if (produced == 0) {
			Q_EMIT output->stateChanged(QAudio::IdleState);
			qCDebug(STOPMOTION_LOG, "out of sound");
			return 0;
		}
		convert(mix.data(), produced * channels, data);
		qCDebug(STOPMOTION_LOG, "Sent %d frames", produced);
		return qint64(produced) * bytesPerFrame;
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
};

QtAudioDriver::QtAudioDriver() {
	const QAudioDevice device = QMediaDevices::defaultAudioOutput();
	impl = std::make_unique<Impl>(device, chooseFormat(device));
}

QtAudioDriver::~QtAudioDriver() {
}

void QtAudioDriver::play() {
}

void QtAudioDriver::playInThread() {
	impl->play();
}

void QtAudioDriver::addAudioFile(std::shared_ptr<AudioFormat> audioFile) {
	impl->addSound(std::move(audioFile));
}

bool QtAudioDriver::initialize() {
	return true;
}

void QtAudioDriver::shutdown() {
	impl->stop();
}

#include "qtaudiodriver.moc"

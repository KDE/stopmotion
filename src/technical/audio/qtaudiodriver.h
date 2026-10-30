/*
 * SPDX-FileCopyrightText: 2017 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef QTAUDIODRIVER_H
#define QTAUDIODRIVER_H

#include "audiodriver.h"

class AudioFormat;

class QtAudioDriver : public AudioDriver {
	class Impl;
	Impl* impl;
public:
	QtAudioDriver();
	~QtAudioDriver() override;
	void play() override;
	void playInThread() override;
	void addAudioFile(AudioFormat *audioFile) override;
	bool initialize() override;
	void shutdown() override;
};

#endif

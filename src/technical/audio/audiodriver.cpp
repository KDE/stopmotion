/***************************************************************************
 *   Copyright (C) 2014 by Linuxstopmotion contributors;                   *
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

#include "audiodriver.h"

#include "src/foundation/logger.h"
#include "audioformat.h"

#include <vector>
#include <pthread.h>
#include <unistd.h>

// Help function for starting a new thread
extern "C" void* startupThread(void *arg)
{
	AudioDriver* ad = reinterpret_cast<AudioDriver*>(arg);
	ad->play();
	pthread_exit(0);
}

class WrappedAudioDriver : public AudioDriver {
	SimpleAudioDriver* simple;
	volatile bool stopPlaying;
	char audioBuffer[4096];
	std::vector<AudioFormat*> audioFiles;
	std::vector<pthread_t> audioThreads;
public:
	WrappedAudioDriver(SimpleAudioDriver* simpleDriver)
		: simple(simpleDriver), stopPlaying(true) {
	}
	~WrappedAudioDriver() {
		shutdown();
		delete simple;
	}
	void play() {
		AudioFormat *af = audioFiles.front();
		audioFiles.erase(audioFiles.begin());
		if ( af->open() != -1 ) {
			while (!stopPlaying) {
				int written = af->fillBuffer(audioBuffer, sizeof(audioBuffer));
				if (written <= 0)
					break;
				simple->play(audioBuffer, written);
			}
			af->close();
		}
	}
	void playInThread() {
		pthread_t audioThread;
		pthread_create(&audioThread, NULL, startupThread, this);
		audioThreads.push_back(audioThread);
	}
	void addAudioFile(AudioFormat *audioFile) {
		audioFiles.push_back(audioFile);
	}
	bool initialize() {
		if (!stopPlaying) {
			// already initialized
			return true;
		}
		Logger::get().logDebug("Initializing the sound device");
		bool r = simple->initialize();
		if (r) {
			stopPlaying = false;
			Logger::get().logDebug("Sound device initialization successful");
		} else {
			Logger::get().logWarning("Sound device initialization failed");
		}
		return r;
	}
	void shutdown() {
		if (stopPlaying) {
			// already stopped
			return;
		}
		Logger::get().logDebug("Shutting down the audio device");
		stopPlaying = true;
		// Wait for the threads to terminate
		unsigned int numElem = audioThreads.size();
		for (unsigned int i = 0; i < numElem; ++i) {
			pthread_join(audioThreads[i], NULL);
		}
		audioThreads.clear();
		numElem = audioFiles.size();
		for (unsigned int i = 0; i < numElem; ++i) {
			// Just to ensure that the file is closed
			audioFiles[i]->close();
		}
		audioFiles.clear();
		simple->shutdown();
	}
};

AudioDriver::~AudioDriver() {
}

AudioDriver* SimpleAudioDriver::makeAudioDriver(
		SimpleAudioDriver* simpleDriver) {
	return new WrappedAudioDriver(simpleDriver);
}

SimpleAudioDriver::~SimpleAudioDriver() {
}

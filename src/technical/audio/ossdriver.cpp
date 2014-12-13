/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#include "ossdriver.h"

#include "audioformat.h"
#include "src/foundation/logger.h"

#include <sys/ioctl.h>
#if defined(__linux__)
#include <linux/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>

OSSDriver::OSSDriver(const char *device) : audioFD(-1), audioDevice(0) {
	audioDevice = new char[strlen(device) + 1];
	strcpy(audioDevice, device);
}

OSSDriver::~OSSDriver() {
	delete [] audioDevice;
	audioDevice = NULL;
	if (audioFD != -1) {
		close(audioFD);
	}
}

void OSSDriver::play(const char* buffer, int bytes) {
	if (audioFD != -1 && audioFD != EBUSY) {
		while (0 < bytes) {
			bytes -= write(audioFD, buffer, bytes);
		}
	}
}

bool OSSDriver::initialize() {
	Logger::get().logDebug("Initializing OSS device");
	// Opens the device write only and sets the flag to non-blocking.
	// This means that the call imidiately returns with a return value
	audioFD = open(audioDevice, O_WRONLY | O_NONBLOCK, 0);
	if (audioFD == -1 || audioFD == EBUSY) {
		Logger::get().logWarning("Cannot open device %s: %s\n", audioDevice, strerror(errno));
		return false;
	}
	// Turns on blocking (disables the non-blocking flag)
	fcntl(audioFD, F_SETFL,0);
	// Storing the request and its argument
	unsigned int initValues[2][4] = {
		{SNDCTL_DSP_RESET, SNDCTL_DSP_SETFMT,
				SOUND_PCM_WRITE_CHANNELS, SOUND_PCM_WRITE_RATE},
		{1, AFMT_S16_LE, 2, 44100}
	};
	for (int i = 0; i < 4; ++i) {
		// Initializes the audio device
		if ( !setIoctl(static_cast<int>(initValues[0][i]),
				static_cast<int>(initValues[1][i])) ) {
			close(audioFD);
			audioFD = -1;
			return false;
		}
	}
	return true;
}

bool OSSDriver::setIoctl(int request, int param) {
	int p = param;
	int ret = ioctl(audioFD, request, &p);
	if (ret == -1) {
		Logger::get().logWarning("Initialization of OSS device failed: %s\n", strerror(errno));
		return false;
	}
	else if (p != param) {
		Logger::get().logWarning("OSS: Device has not support for requested argument %d\n", param);
		return false;
	}
	return true;
}

void OSSDriver::shutdown() {
	if (audioFD != -1) {
		Logger::get().logDebug("Shuttting down the OSS audio device");
		close(audioFD);
		audioFD = -1;
	}
}

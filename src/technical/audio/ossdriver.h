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
#ifndef OSSDRIVER_H
#define OSSDRIVER_H

#include "audiodriver.h"
#include <vector>
#include <pthread.h>
#include <unistd.h>


/**
 * OSS audio driver. This takes care of initializing the registered audio
 * device with differents parameters. The device will then by ready to
 * to play sound if the initializing was successfull.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class OSSDriver : public SimpleAudioDriver {
public:
	/**
	 * Registers the given device.
	 * @param device the device to be used
	 */
	OSSDriver(const char *device);
	~OSSDriver();
	void play(const char* buffer, int bytes);
	bool initialize();
	void shutdown();
private:
	/** Descriptor to the audio device. */
	int audioFD;
	/** Pointer to the registered audio device. */
	char *audioDevice;
	/**
	 * Does a ioctl call with the given parameters. It checks if the call
	 * was successfully done.
	 * @param request the request
	 * @param param parameter to the request
	 * @return true on success, false otherwise
	 */
	bool setIoctl(int request, int param);
};

#endif

/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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
#include "src/technical/util.h"

#include "src/technical/libng/grab-ng.h"

#include <ext/stdio_filebuf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <cassert>
#include <istream>


using namespace std;


const char* Util::checkCommand(const char *command)
{
	assert(command != 0);
	
	int len = 7 + strlen(command);
	char tmp[len];
	snprintf(tmp, len, "which %s", command);
	
	FILE *fp = popen(tmp, "r");
	__gnu_cxx::stdio_filebuf<char> buf(fp, ios::in);
	istream bufStream(&buf);
	
	string line = "";
	getline(bufStream, line);
	if (line != "") {
		char *path = new char[line.length() + 1];
		strcpy(path, line.c_str());
		return path;
	}
	
	return 0;
}


const vector<GrabberDevice> Util::getGrabberDevices()
{
	// Ensures ng_init() is called once
	static int count = 0;
	if (count++ == 0) {
		ng_init();
	}
	
	vector<GrabberDevice> devices;
	const struct ng_vid_driver *driver = 0;
	void *handle = 0;
	struct stat st;
	int fh    = -1;
	int flags = -1;

	for (int i = 0; ng_dev.video_scan[i] != 0; ++i) {
		if (lstat(ng_dev.video_scan[i], &st) == -1) {
			if (errno == ENOENT) {
				continue;
			}
#ifndef NO_DEBUG
			fprintf(stderr,"%s: %s\n",ng_dev.video_scan[i],strerror(errno));
#endif
			continue;
		}

		fh = open(ng_dev.video_scan[i], O_RDWR);
		if (fh == -1) {
			if (ENODEV == errno) {
				continue;
			}
#ifndef NO_DEBUG
			fprintf(stderr,"%s: %s\n",ng_dev.video_scan[i],strerror(errno));
#endif
			continue;
		}
		close(fh);

		driver = ng_vid_open(ng_dev.video_scan[i], 0, 0, 0, &handle);
		if (driver == 0) {
#ifndef NO_DEBUG
			fprintf(stderr,"%s: initialization failed\n",ng_dev.video_scan[i]);
#endif
			continue;
		}

		flags = driver->capabilities(handle);
		if (flags & CAN_CAPTURE) {
			GrabberDevice d;
			d.device = ng_dev.video_scan[i];
			if (driver->get_devname) {
				d.name = driver->get_devname(handle);
			}
			d.type = driver->name;
			devices.push_back(d);
		}

		driver->close(handle);
	}

	vector<GrabberDevice>(devices).swap(devices);
	return devices;
}


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
#include <cassert>
#include <istream>
#include <ftw.h>

using namespace std;

FileLinkException::FileLinkException(const char* message) {
	snprintf(msg, sizeof(msg) - 1, "link error: %s", message);
	msg[sizeof(msg) - 1] = '\0';
}

const char* FileLinkException::what() const _GLIBCXX_USE_NOEXCEPT {
	return msg;
}

DirectoryCreationException::DirectoryCreationException(const char* path) {
	snprintf(buffer, sizeof(buffer), "Failed to create directory (%s)", path);
}

const char* DirectoryCreationException::what() const
		_GLIBCXX_USE_NOEXCEPT {
	return buffer;
}

namespace {
void handleFileLinkError() {
	switch(errno) {
	case EACCES:
		// do not have permissions to write (maybe wrong file system type)
	case EXDEV:
		// cannot link across file systems
	case EMLINK:
		// too many links already
		// ...worth trying a different method.
		return;
	case EEXIST:
		throw FileLinkException("File already exists");
	case ENOENT:
		throw FileLinkException("File does not exist to be linked to");
	case ENOSPC:
		throw FileLinkException("Out of disk space");
	case EPERM:
		throw FileLinkException("Permission denied");
	case EROFS:
		throw FileLinkException("Cannot write to a read-only file system");
	case EIO:
		throw FileLinkException("I/O error");
	default:
		throw FileLinkException("unknown error");
	}
}

int removeFileOrDirectory(const char *path, const struct stat *,
		int flag, struct FTW *info) {
	switch (flag) {
	case FTW_D:
		if (info->level != info->base) {
			if (0 != rmdir(path))
				return FTW_STOP;
		}
		break;
	default:
		if (0 != unlink(path))
			return FTW_STOP;
		break;
	}
	return FTW_CONTINUE;
}

const char* endOfArgument(const char* in) {
	enum CharClass {
		normalChar,
		backslashChar,
		squoteChar,
		dquoteChar,
		space
	};
	enum State {
		normal,
		backslash,
		squote,
		dquote,
		dqbackslash,
		end
	} state = normal;
	/* transition[state][charClass] */
	static const State transition[5][5] = {
			{ normal, backslash, squote, dquote, end }, // normal
			{ normal, normal, normal, normal, normal }, // backslash
			{ squote, squote, normal, squote, squote }, // squote
			{ dquote, dqbackslash, dquote, normal, dquote }, // dquote
			{ dquote, dquote, dquote, dquote, dquote } // dqbackslash
	};
	char c = *in;
	while (c != '\0' && state != end) {
		CharClass cc = c == '\\'? backslashChar
				: c == '\''? squoteChar
						: c == '"'? dquoteChar
								: c == ' '? space
										: normalChar;
		state = transition[state][cc];
		++in;
		c = *in;
	}
	return in;
}

}

bool Util::checkCommand(std::string* pathOut, const char* command) {
	assert(command != 0);
	std::string which("which ");
	const char* commandEnd = endOfArgument(command);
	which.append(command, commandEnd);
	FILE *fp = popen(which.c_str(), "r");
	__gnu_cxx::stdio_filebuf<char> buf(fp, ios::in);
	istream bufStream(&buf);
	std::string dummy;
	if (!pathOut)
		pathOut = &dummy;
	*pathOut = "";
	std::getline(bufStream, *pathOut);
	while (bufStream.good()) {
		std::getline(bufStream, dummy);
	}
	bool bad = bufStream.bad();
	int status = pclose(fp);
	int exitStatus = WEXITSTATUS(status);
	return !bad && exitStatus < 2;
}


const vector<GrabberDevice> Util::getGrabberDevices() {
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

bool Util::copyFile(const char *destFileName, const char *srcFileName) {
    assert(destFileName != 0);
    assert(srcFileName != 0);

    FILE *src = fopen(srcFileName, "rb");
    if (!src) {
        fprintf(stderr, "Failed to open '%s' for reading: %s\n", srcFileName, strerror(errno));
        return false;
    }

    FILE *dest = fopen(destFileName, "wb");
    if (!dest) {
        fprintf(stderr, "Failed to open '%s' for writing: %s\n", destFileName, strerror(errno));
        fclose (src);
        return false;
    }

    char buf[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, bytesRead, dest) != bytesRead) {
            fprintf (stderr, "Error while writing '%s': %s\n", destFileName, strerror(errno));
            break;
        }
    }

    if (bytesRead == 0 && ferror(src))
        fprintf(stderr, "Error while reading from file '%s': %s\n", srcFileName, strerror(errno));

    fclose(src);
    fclose(dest);

    return true;
}

void Util::linkOrCopyFile(const char* newName, const char* oldName) {
	if (-1 == link(oldName, newName)) {
		handleFileLinkError();
		if (-1 == symlink(oldName, newName)) {
			handleFileLinkError();
			if (!Util::copyFile(newName, oldName))
				throw FileLinkException("Could not copy file");
		}
	}
}

bool Util::removeDirectoryContents(const char* path) {
	static const int maxDescripotorsToConsume = 8;
	return FTW_STOP != nftw(path, removeFileOrDirectory,
			maxDescripotorsToConsume, FTW_PHYS | FTW_DEPTH | FTW_ACTIONRETVAL);
}

void Util::ensurePathExists(const char* path) {
	if (0 == access(path, F_OK))
		return;
	std::string copy(path);
	copy.c_str();  // ensure terminating '\0'
	char* parent = &copy[0];
	char* end = strrchr(parent, '/');
	if (end && end[1] == '\0') {
		// there is a trailing '/', so let's remove it and try again.
		*end = '\0';
		end = strrchr(parent, '/');
	}
	if (end) {
		*end = '\0';
		ensurePathExists(parent);
		if (mkdir(path, 0755) < 0)
			throw DirectoryCreationException(path);
	} else {
		throw DirectoryCreationException(path);
	}
}

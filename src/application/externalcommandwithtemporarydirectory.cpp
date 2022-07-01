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

#include "externalcommandwithtemporarydirectory.h"
#include "externalcommand.h"

#include <QString>

#include <src/technical/util.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <memory>

class TemporaryDirectoryCreationException : public std::exception {
	char msg[100];
public:
	TemporaryDirectoryCreationException(const char* reason) {
		strncpy(msg, reason, sizeof(msg) - 1);
		msg[sizeof(msg) - 1] = '\0';
	}
	const char* what() const throw() {
		return msg;
	}
};

class TemporaryDirectory {
	std::string path;
	const char* pptr;
	bool tryTemplate(const char* var, const char* baseDir) {
		path = "";
		if (var) {
			const char* v = getenv(var);
			if (!v)
				return false;
			path += v;
		}
		if (baseDir)
			path += baseDir;
		path += "/lsmXXXXXX";
		pptr = path.c_str();
		return mkdtemp(&path[0]);
	}
public:
	//TODO there's got to be a better way of making a temporary directory
	TemporaryDirectory() : pptr(0) {
		if (!tryTemplate("TMPDIR", 0)
				&& !tryTemplate(0, "/tmp")
				&& !tryTemplate("HOME", "/.stopmotion")) {
			throw TemporaryDirectoryCreationException(
					"Failed to create temporary directory");
		}
	}
	~TemporaryDirectory() {
		Util::removeDirectoryContents(pptr);
		rmdir(pptr);
	}
	const char* getPath() const {
		return pptr;
	}
};

ExternalCommandWithTemporaryDirectory
		::ExternalCommandWithTemporaryDirectory(
		QWidget* parent) :ec(0), td(0) {
	std::unique_ptr<TemporaryDirectory> ttd(new TemporaryDirectory);
	ec = new ExternalCommand(parent);
	td = ttd.release();
}

ExternalCommandWithTemporaryDirectory
		::~ExternalCommandWithTemporaryDirectory() {
	delete ec;
	ec = 0;
	delete td;
	td = 0;
}

void ExternalCommandWithTemporaryDirectory::run(const QString& command) {
	ec->run(command);
}

void ExternalCommandWithTemporaryDirectory::show() {
	ec->show();
}

const char* ExternalCommandWithTemporaryDirectory
		::getTemporaryDirectoryPath() const {
	return td->getPath();
}

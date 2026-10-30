/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
	const char* what() const noexcept override {
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
	TemporaryDirectory() : pptr(nullptr) {
		if (!tryTemplate("TMPDIR", nullptr)
				&& !tryTemplate(nullptr, "/tmp")
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
		QWidget* parent) :ec(nullptr), td(nullptr) {
	std::unique_ptr<TemporaryDirectory> ttd(new TemporaryDirectory);
	ec = new ExternalCommand(parent);
	td = ttd.release();
}

ExternalCommandWithTemporaryDirectory
		::~ExternalCommandWithTemporaryDirectory() {
	delete ec;
	ec = nullptr;
	delete td;
	td = nullptr;
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

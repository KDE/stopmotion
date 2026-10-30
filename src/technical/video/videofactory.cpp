/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "videofactory.h"

#include "src/domain/animation/animationimpl.h"
#include "src/technical/util.h"
#include "src/technical/video/videoencoder.h"
#include "src/domain/filenamevisitor.h"
#include "src/foundation/logger.h"
#include "src/application/externalcommandwithtemporarydirectory.h"

#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>

#include <QString>

using namespace std;

VideoFactory::VideoFactory(const AnimationImpl *animation)
		: anim(animation) {
}


VideoFactory::~VideoFactory() {
}

class FileCopier : public FileNameVisitor {
	int index;
	const char* dir;
public:
	FileCopier(const char* dirPath) : index(0), dir(dirPath) {
	}
	~FileCopier() override {
	}
	void visitImage(const char* p) override {
		std::ostringstream ss;
		ss << dir << "/" << std::setw(6) << std::setfill('0') << index;
		std::string path = ss.str();
		const char* ext = strrchr(p, '.');
		if (ext)
			path.append(ext);
		Util::linkOrCopyFile(path.c_str(), p);
		++index;
	}
	void visitSound(const char*) override {
	}
};

bool replaceText(string& text, const char* searchFor, const char* replaceWith) {
	int index = text.find(searchFor);
	if (index < 0)
		return false;
	int replaceLength = strlen(replaceWith);
	while (0 <= index) {
		text.replace(index, strlen(searchFor), replaceWith);
		index = text.find(searchFor, index + replaceLength);
	}
	return true;
}

std::string integerToString(int n) {
	std::stringstream stream;
	stream << n;
	return stream.str();
}

const char* VideoFactory::createVideoFile(VideoEncoder *encoder,
		int playbackSpeed) {
	std::string startCommand = encoder->getStartCommand();
	std::unique_ptr<ExternalCommandWithTemporaryDirectory> ec(
			new ExternalCommandWithTemporaryDirectory());
	const char* tmpDir = ec->getTemporaryDirectoryPath();
	if ( !startCommand.empty() ) {
		replaceText(startCommand, "$IMAGEPATH", tmpDir);
		if (!replaceText(startCommand, "$VIDEOFILE", encoder->getOutputFile())) {
			return nullptr;
		}
		replaceText(startCommand, "$opt", "");
		std::string playbackSpeedString = integerToString(playbackSpeed);
		replaceText(startCommand, "$FRAMERATE", playbackSpeedString.c_str());
		Logger::get().logDebug("Copying frames into temporary directory %s",
				tmpDir);
		FileCopier copier(tmpDir);
		anim->accept(copier);
		sync();
		ec->show();
		ec->run(QString::fromLocal8Bit(startCommand.c_str()));
		ec.release();
		return encoder->getOutputFile();
	}
	return nullptr;
}

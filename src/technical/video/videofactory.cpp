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

#include "videofactory.h"

#include "src/domain/animation/animationimpl.h"
#include "src/presentation/frontends/frontend.h"
#include "src/technical/util.h"
#include "src/technical/video/videoencoder.h"
#include "src/domain/filenamevisitor.h"
#include "src/foundation/logger.h"
#include "src/application/externalcommandwithtemporarydirectory.h"

#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>

#include <QString>

using namespace std;

VideoFactory::VideoFactory(const AnimationImpl *animation, Frontend *frontend)
		: anim(animation), frontend(frontend) {
}


VideoFactory::~VideoFactory() {
}

class FileCopier : public FileNameVisitor {
	int index;
	const char* dir;
public:
	FileCopier(const char* dirPath) : index(0), dir(dirPath) {
	}
	~FileCopier() {
	}
	void visitImage(const char* p) {
		std::ostringstream ss;
		ss << dir << "/" << std::setw(6) << std::setfill('0') << index;
		std::string path = ss.str();
		const char* ext = strrchr(p, '.');
		if (ext)
			path.append(ext);
		Util::linkOrCopyFile(path.c_str(), p);
		++index;
	}
	void visitSound(const char*) {
	}
};

const char* VideoFactory::createVideoFile(VideoEncoder *encoder) {
	string startCommand = encoder->getStartCommand();
	std::auto_ptr<ExternalCommandWithTemporaryDirectory> ec(
			new ExternalCommandWithTemporaryDirectory());
	const char* tmpDir = ec->getTemporaryDirectoryPath();
	if ( !startCommand.empty() ) {
		int index = startCommand.find("$IMAGEPATH");
		if (index != -1) {
			startCommand.replace(index, strlen("$IMAGEPATH"), tmpDir);
		}
		index = startCommand.find("$VIDEOFILE");
		if (index != -1) {
			if ( encoder->getOutputFile() ) {
				string outputFile = encoder->getOutputFile();
				startCommand.replace(index, strlen("$VIDEOFILE"), outputFile);
			}
			else {
				return NULL;
			}
		}
		index = startCommand.find("$opt");
		if (index != -1) {
			startCommand.replace(index, strlen("$opt"), "");
		}
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
	return NULL;
}

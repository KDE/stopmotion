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

#include "src/technical/video/videoencoder.h"

#include "src/technical/util.h"

#include <cstdlib>
#include <cstring>


VideoEncoder::VideoEncoder()
{
	startCommand = nullptr;
	stopCommand = nullptr;
	outputFile = nullptr;
}


VideoEncoder::~ VideoEncoder()
{
	delete [] startCommand;
	startCommand = nullptr;
	delete [] stopCommand;
	stopCommand = nullptr;
	delete [] outputFile;
	outputFile = nullptr;
}


const char* VideoEncoder::getStartCommand()
{
	return startCommand;
}


const char* VideoEncoder::getStopCommand()
{
	return stopCommand;
}


const char * VideoEncoder::getOutputFile()
{
	return outputFile;
}


void VideoEncoder::setStartCommand(const char* command)
{
	if (startCommand) {
		delete [] startCommand;
		startCommand = nullptr;
	}
	startCommand = new char[strlen(command) + 1];
	strcpy(startCommand, command);
}


void VideoEncoder::setStopCommand(const char* command)
{
	if (stopCommand) {
		delete [] stopCommand;
		stopCommand = nullptr;
	}
	stopCommand = new char[strlen(command) + 1];
	strcpy(stopCommand, command);
}


void VideoEncoder::setOutputFile(const char* file)
{
	if (outputFile) {
		delete [] outputFile;
		outputFile = nullptr;
	}
	outputFile = new char[strlen(file) + 1];
	strcpy(outputFile, file);
}


bool VideoEncoder::isValid() {
	return Util::checkCommand(nullptr, startCommand);
}

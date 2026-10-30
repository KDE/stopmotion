/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "src/technical/video/videoencoder.h"

#include "src/technical/util.h"



VideoEncoder::VideoEncoder()
{
}


VideoEncoder::~ VideoEncoder()
{
}


const char* VideoEncoder::getStartCommand()
{
	return startCommand.c_str();
}


const char* VideoEncoder::getStopCommand()
{
	return stopCommand.c_str();
}


const char * VideoEncoder::getOutputFile()
{
	return outputFile.c_str();
}


void VideoEncoder::setStartCommand(const char* command)
{
	startCommand = command;
}


void VideoEncoder::setStopCommand(const char* command)
{
	stopCommand = command;
}


void VideoEncoder::setOutputFile(const char* file)
{
	outputFile = file;
}


bool VideoEncoder::isValid() {
	return Util::checkCommand(nullptr, startCommand.c_str());
}

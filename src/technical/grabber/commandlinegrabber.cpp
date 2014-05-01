/***************************************************************************
 *   Copyright (C) 2005-2008 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad*
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#include "src/technical/grabber/commandlinegrabber.h"
#include "src/technical/util.h"

#include <cstdlib>
#include <sstream>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <cstring>

using namespace std;


CommandLineGrabber::CommandLineGrabber(const char* filePath, bool isProcess)
		: ImageGrabber(filePath, isProcess)
{
	isInitSuccess = false;
	this->prePoll = "";
	this->startProcess = "";
	this->stopProcess = "";
}


bool CommandLineGrabber::setPrePollCommand(const char *command) 
{
	// This happens if the user doesn't uses pre poll
	if ( strcmp(command, "") == 0) {
		return true;
	}
	
	prePoll = parseCommand(command);
	if (prePoll != "") {
		return true;
	}
	return false;
}


bool CommandLineGrabber::setStartCommand(const char *command)
{
	// This happens if the user doesn't uses start command
	if ( strcmp(command, "") == 0) {
		return true;
	}
	
	startProcess = parseCommand(command);
	if (startProcess != "") {
		return true;
	}
	return false;
}


bool CommandLineGrabber::setStopCommand(const char *command)
{
	stopProcess	= parseCommand(command);
	return true;
}


bool CommandLineGrabber::init()
{
	if (isProcess) {
		if (startProcess != "") {
			Logger::get().logDebug("Attemting to start process");
			system(startProcess.c_str());
			
		}
		else {
			return false;
		}
	}
	return true;
}


bool CommandLineGrabber::tearDown()
{
	Logger::get().logDebug("Attemting to shutt down process");
	if (stopProcess != "") {
		system( stopProcess.c_str() );
		return true;
	}
	return false;
}


bool CommandLineGrabber::grab()
{
	if ( prePoll.empty() || system(prePoll.c_str()) != 0 ) {
		isInitSuccess = false;
		return false;
	}
	return true;
}


string CommandLineGrabber::parseCommand(const char * command)
{
	string tmp = command;
	int spaceIdx = tmp.find(" ", 0);
	std::string commandName = tmp.substr(0, spaceIdx);
	std::string path;
	if (Util::checkCommand(&path, commandName.c_str())) {
		tmp.replace(0, spaceIdx, path);
		int index = tmp.find("$IMAGEFILE");
		if (index != -1) {
			tmp.replace(index, strlen("$IMAGEFILE"), string(filePath));
		}
		return tmp;
	}
	return "";
}


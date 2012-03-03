/***************************************************************************
 *   Copyright (C) 2005 by fredrik                                         *
 *   fredrik@Elanor                                                        *
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
#include "commandlinegrabber.h"

#include <sstream>
#include <sys/types.h>
#include <signal.h>

#include <iostream>

using namespace std;


CommandLineGrabber::CommandLineGrabber(char* filePath, const char* prePoll,
		const char* startProcess, const char* stopProcess, bool isProcess)
		: ImageGrabber(filePath, isProcess)
{
	isInitSuccess = false;
	this->prePoll 		= parseCommand(prePoll);
	this->startProcess 	= parseCommand(startProcess);
	this->stopProcess	= parseCommand(stopProcess);
}


bool CommandLineGrabber::init()
{
	if(isProcess) {
		if(startProcess != "") {
			Logger::get().logDebug("Attemting to start process");
			//Can't test this this way as the command line for starting the
			//process as a vgrabbj deamon returns 256 no matter if it works or if it
			//fails. Needs to be fixed some other way, and i think i have an idea.
			//if ( system(startProcess.c_str()) != 0 ) {
			//	return false;
			//}
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
	if(stopProcess != "") {
		system( stopProcess.c_str() );
	}
	else {
		return false;
	}
	return true;
}


bool CommandLineGrabber::grab()
{
	if( system( prePoll.c_str() ) != 0 ) {
		isInitSuccess = false;
		return false;
	}
	return true;
}


string CommandLineGrabber::parseCommand( const char * command )
{
	string tmp = command;
	
	int location = (int)tmp.find("(DEFAULTPATH)");
	
	if(location != -1) {
		tmp.replace(location, 13, string(filePath));
	}
	return tmp;
}




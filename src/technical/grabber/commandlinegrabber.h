/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#ifndef COMMANDLINEGRABBER_H
#define COMMANDLINEGRABBER_H

#include "src/config.h"

#include "imagegrabber.h"

#include <string>
using std::string;

/**
 * Abstract class for the different video grabbers used by the VideoView
 * widgets.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class CommandLineGrabber : public ImageGrabber
{
public:
	
	/**
	 * Initializes the member variables.
	 * @param filePath path to the output file grabbed from a device
	 * @param prePoll the pre-poll command to be runned before grabbing
	 * @param startProcess command line for starting the process
	 * @param stopProcess command line for stopping the process
	 * @param isProcess true if the process is running in deamon mode, false otherwise
	 */
	CommandLineGrabber(char* filePath, const char* prePoll, const char* startProcess, 
			const char* stopProcess, bool isProcess = false);
	~CommandLineGrabber() {};
	
	/**
	 * Starts the grabber if it is marked to be runned in deamon mode.
	 * @return true on success, false otherwise
	 */
	bool init();
	
	/**
	 * Grabs one picture from the device.
	 * @return true on success, false otherwise
	 */
	bool grab();
	
	/**
	 * Shut downs the grabber process either if it is runned in deamon
	 * mode or "single grab" mode.
	 * @return true on success, false otherwise
	 */
	bool tearDown();
private:
	string prePoll;
	string startProcess;
	string stopProcess;
	bool isInitSuccess;
	
	string parseCommand(const char* command);
};

#endif

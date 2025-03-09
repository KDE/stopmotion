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
class CommandLineGrabber : public ImageGrabber {
public:
	/**
	 * Initializes the member variables.
	 * @param filePath path to the output file grabbed from a device
	 * @param prePoll the pre-poll command to be run before grabbing
	 * @param startProcess command line for starting the process
	 * @param stopProcess command line for stopping the process
	 * @param isProcess true if the process is running in daemon mode, false otherwise
	 */
	CommandLineGrabber(const char* filePath);
	~CommandLineGrabber() override;
	bool setPrePollCommand(const char *command) override;
	bool setStartCommand(const char *command) override;
	bool setStopCommand(const char *command) override;
	bool isGrabberProcess() override;
	
	/**
	 * Starts the grabber if it is marked to be run in daemon mode.
	 * @return true on success, false otherwise
	 */
	bool init() override;
	
	/**
	 * Grabs one picture from the device.
	 * @return true on success, false otherwise
	 */
	bool grab() override;
	
	/**
	 * Shut downs the grabber process either if it is run in daemon
	 * mode or "single grab" mode.
	 * @return true on success, false otherwise
	 */
	bool tearDown() override;

private:
	string prePoll;
	string startProcess;
	string stopProcess;
	bool isInitSuccess;
	
	string parseCommand(const char* command);
};

#endif

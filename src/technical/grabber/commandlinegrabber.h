/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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

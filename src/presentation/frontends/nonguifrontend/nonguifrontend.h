/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef NONGUI_H
#define NONGUI_H

#include <string>

#include "src/config.h"
#include "src/presentation/frontends/frontend.h"

class DomainFacade;
class UiException;

/**
 * Frontend for using the program through command line options.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class NonGUIFrontend : public Frontend
{
public:
    /**
     * Constructs and initializes the object.
     * @param facadePtr pointer to the domain facade
     */
    NonGUIFrontend(DomainFacade *facadePtr);
	~NonGUIFrontend() override;
	
	/**
	 * The run function for starting the application.
	 * @param argc the argc argument from the environment through main.
	 * @param argv the argv arguments from the environment through main.
	 * @return the return status on exit
	 */
	int run(int argc, char **argv) override;

	/**
	 * Loads the preferences, creating default preferences if there are none
	 * yet. Must be called before the preferences are used.
	 * @throws UiException if the preferences file exists but cannot be read.
	 */
	void initializePreferences();
	
	void showProgress(ProgressMessage message, int numOperations) override;
	void hideProgress() override;
	void updateProgress(int numOperationsDone) override;
	void setProgressInfo(const char *infoText);
	bool isOperationAborted() override;
	
	/**
	 * Function for processing events. This is useful on timeconsuming
	 * operations which aren't running in separate processes or threads.
	 * This function is actually not needed in this frontend.
	 */
	void processEvents() override;
	bool askQuestion(Question question) override;
	int runExternalCommand(const char *command) override;
	void reportWarning(const char *message) override;
	void handleException(UiException&) override;
	
private:
	DomainFacade *facadePtr;
	
	int parseArguments(int argc, char **argv);
	void addFrames(const char *directory);
	void save(const char *projectFile);
	/**
	 * Sets {@a out} to be the absolute path of {@a path}.
	 * @param[out] out Receives the path returned.
	 * @param[in] path The path to make absolute.
	 * @param[in] isDirectory If true, a trailing slash is added if missing.
	 * Must be false for paths of files.
	 */
	void getAbsolutePath(std::string& out, const char *path, bool isDirectory);
	int checkFiles(const char *directory);
};

#endif

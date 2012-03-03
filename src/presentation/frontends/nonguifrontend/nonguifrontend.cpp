/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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
#include "nonguifrontend.h"

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


NonGUIFrontend::NonGUIFrontend(DomainFacade *facadePtr)
		: facadePtr(facadePtr)
{
}


NonGUIFrontend::~ NonGUIFrontend()
{
}


int NonGUIFrontend::run(int argc, char **argv) 
{
	printf("NonGUIFrontend starts running ...\n");
	return parseArguments(argc, argv);
}


void NonGUIFrontend::showProgress(const char *infoText, unsigned int) 
{
	// sets the stdout to be unbuffered
	setvbuf(stdout, NULL, _IONBF,0);
	printf("%s", infoText);
}


void NonGUIFrontend::hideProgress() 
{
	printf("\nOperation finished!\n");
	// sets the stdout the be buffered
	setvbuf (stdout, NULL , _IOFBF , 1024);
}


void NonGUIFrontend::updateProgress(int) 
{
	printf(".");
}


void NonGUIFrontend::setProgressInfo(const char *infoText) 
{
	printf("\n%s\n", infoText);
}


bool NonGUIFrontend::isOperationAborted() { return false; }
void NonGUIFrontend::processEvents() {}


int NonGUIFrontend::parseArguments(int argc, char **argv)
{
	while (1) {
		static struct option longOptions[] =
			{
				{"nonGUI", 0, 0, 0},
				{"addFrames", 1, 0, 'a'},
				{"capture", 1, 0, 'c'},
				{"save", 1, 0, 's'},
				{"export", 0, 0, 'e'},
				{0, 0, 0, 0}
			};
		
		/* getopt_long  stores the option index here. */
		int optionIndex = 0;
		int c = getopt_long(argc, argv, "a:c:s:e:", longOptions, &optionIndex);
	
		/* Detect the end of the options. */
		if (c == -1) {
			break;
		}
	
		switch (c) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				break;
	
			case 'a':
				//printf ("option --addFrames with arument %s\n", optarg);
				addFrames(optarg);
				break;
	
			case 'c':
				printf ("option --capture with arument %s\n", optarg);
				break;
	
			case 'e':
				printf ("option --export with arument %s\n", optarg);
				break;
	
			case 's':
				//printf ("option --save with arument %s\n", optarg);
				save(optarg);
				break;
	
			case '?':
				/* getopt_long  already printed an error message. */
				break;
	
			default:
				return 1;
		}
	}
	return 0;
}


void NonGUIFrontend::addFrames(const char *directory)
{
	// returns a absoulte path which is allocated with new[]
	const char *dir = getAbsolutePath(directory);

	DIR *dp = opendir(dir);
	if (dp) {
		vector<char*> frames;
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			char *fileName = new char[256];
			snprintf(fileName, 256, "%s%s", dir, ep->d_name);
			stat(fileName, &st);
			// is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0 && strstr(fileName, "snd") == NULL ) {
				frames.push_back(fileName);
			}
			else {
				delete [] fileName;
			}
			fileName = NULL;
		}
		closedir(dp);
		
		vector<char*>(frames).swap(frames);
		facadePtr->addFrames(frames);

		unsigned int numElem = frames.size();
		for (unsigned int i = 0; i < numElem; ++i) {
			delete [] frames[i];
		}
		
		char tmpDir[256] = {0};
		snprintf(tmpDir, 256, "%s/.stopmotion/tmp/", getenv("HOME"));
		int success = checkFiles(tmpDir);
		printf("Successfully copied %d files from %s to %s\n", success, dir, tmpDir);
	}
	else {
		fprintf (stderr, "Couldn't open directory: %s; %s\n", dir, strerror (errno));
	}
	
	delete [] dir;
}


void NonGUIFrontend::save(const char *directory)
{
	// returns a absoulte path which is allocated with new[]
	const char *dir = getAbsolutePath(directory);
	facadePtr->saveProject(dir);
	
	char tmp[256] = {0};
	snprintf(tmp, 256, "%simages/", dir);
	int saved = checkFiles(tmp);
	printf("Successfully saved %d files in %s\n", saved, tmp);
	snprintf(tmp, 256, "%s/.stopmotion/tmp/", getenv("HOME"));
	int numFiles = checkFiles(tmp);
	printf("Successfully removed %d files from %s\n", saved - numFiles, tmp);
	
	delete [] dir;
}


const char* NonGUIFrontend::getAbsolutePath(const char *path)
{
	char *tmp = new char[256];
	
	// isn't an absolute path
	if (path[0] != '/') {
		// make it absolute
		snprintf(tmp, 256, "%s/%s", getenv("PWD"), path);
	}
	else {
		strcpy(tmp, path);
	}
	
	struct stat st;
	stat(tmp, &st);
	// if it is a directory
	if ( S_ISDIR(st.st_mode) != 0) {
		int len = strlen(tmp);
		// and doesn't ends with a '/'
		if ( tmp[len - 1] != '/' ) {
			// append a '/'
			snprintf(tmp, 256, "%s/", tmp);
		}
	}
	
	return tmp;
}


int NonGUIFrontend::checkFiles(const char *directory)
{
	int numSuccessFull = 0;
	DIR *dp = opendir(directory);
	
	if (dp) {
		struct dirent *ep;
		struct stat st;
		char tmp[256] = {0};
		
		while ( (ep = readdir(dp)) ) {
			snprintf(tmp, 256, "%s%s", directory, ep->d_name);
			stat(tmp, &st);
			// is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0) {
				++numSuccessFull;
			}
		}
		closedir(dp);
	}
	else {
		fprintf (stderr, "Couldn't open directory: %s; %s\n",directory, strerror (errno));
	}
	
	return numSuccessFull;
}


void NonGUIFrontend::reportError( const char *message, int id )
{
	id = id != 0 && id != 1 ? 0 : id;
	
	if (id == 0) {
		printf("Warning: %s\n", message);
	}
	else {
		printf("Critical: %s\n", message);
	}
}


int NonGUIFrontend::askQuestion(const char *)
{
	return 1;
}


int NonGUIFrontend::runExternalCommand(const char *)
{
	return 1;
}

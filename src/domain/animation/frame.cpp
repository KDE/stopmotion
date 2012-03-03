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
#include "frame.h"

#include "src/technical/audio/oggvorbis.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <libgen.h>

unsigned int Frame::tmpNum   = 0;
unsigned int Frame::trashNum = 0;
unsigned int Frame::soundNr  = 0;
char Frame::tempPath[256]   = {0};
char Frame::trashPath[256]  = {0};


Frame::Frame(const char *filename)
{
	assert(filename != NULL);
	
	sprintf(tempPath, "%s/.stopmotion/tmp/", getenv("HOME"));
	sprintf(trashPath, "%s/.stopmotion/trash/", getenv("HOME"));
	
	int len = strlen(filename) + 1;
	imagePath = new char[len];
	strcpy(imagePath, filename);
	isProjectFile = false;
	
	assert(tempPath  != NULL);
	assert(trashPath != NULL);
	assert(imagePath != NULL);
}


Frame::~Frame()
{
	delete [] imagePath;
	imagePath = NULL;
	
	unsigned int numElem = sounds.size();
	for (unsigned int i = 0; i < numElem; i++) {
		delete sounds[i];
		sounds[i] = NULL;
	}
}


char* Frame::getImagePath()
{
	assert(imagePath != NULL);
	Logger::get().logDebug("Retrieving picture from frame: ");
	Logger::get().logDebug(imagePath);
	return imagePath;
}


/**
 *@todo check audio type (ogg, mp3, wav ...)
 */
int Frame::addSound(const char *filename)
{
	Logger::get().logDebug("Adding sound in frame");
	AudioFormat *f = new OggVorbis();
	int ret = f->setFilename(filename);
	if (ret != 0) {
		delete f;
		return ret;
	}
	sounds.push_back(f);

	stringstream ss;
	ss << "Sound" << soundNr++;
	soundNames.push_back( ss.str() );
	
	return 0;
}


void Frame::removeSound(unsigned int soundNumber)
{
	delete sounds[soundNumber];
	sounds[soundNumber] = NULL;
	
	sounds.erase(sounds.begin() + soundNumber);
}


unsigned int Frame::getNumberOfSounds( )
{
	return sounds.size();
}


vector<AudioFormat*>& Frame::getSounds()
{
	return sounds;
}


void Frame::setSoundName(unsigned int soundNumber, char* soundName)
{
	soundNames[soundNumber] = soundName;
}


char* Frame::getSoundName(unsigned int soundNumber)
{
	return (char*)soundNames[soundNumber].c_str();
}


void Frame::moveToImageDir(const char *directory, unsigned int imgNum)
{
	assert(directory != 0);
	
	char newPath[256]  = {0};
	char filename[12] = {0};
	char tmp[7] = {0};
	
	sprintf(tmp, "%d", imgNum);
	int fileLength = strlen(tmp);
	
	// creates a filename with six characters as total length,
	// empty characters are filled with zeros
	for (int i = 0; i < 6 - fileLength; i++) {
		filename[i] = '0';
	}
	strcat(filename, tmp);

	// gets a pointer to the last occurence of a '.'
	// and appends the extension
	char *dotPtr = strrchr(imagePath,'.');
	strncat (filename, dotPtr, 5);
	
	// creates new image path
	strcpy(newPath, directory);
	strcat(newPath, filename);
	
	// the image is already in the image directory, only renaming
	// is necessary
	if (isProjectFile) {
		char *currP = strdup(imagePath);
		char *newP = strdup(newPath);
		
		int relation = strcmp( basename(currP), basename(newP) );
		if ( relation != 0) {
			char command[512] = {0};
			sprintf(command, "/bin/cp %s %s", imagePath, newPath);
			system(command);
		}
		else {
			rename (imagePath, newPath);
		}
	}
	else {
		int relation = strcmp(imagePath, newPath);
		if (relation != 0) {
			rename(imagePath, newPath);
		}
	}
	
	delete [] imagePath;
	imagePath = new char[strlen(newPath) + 1];
	strcpy(imagePath, newPath);
}


void Frame::copyToTemp()
{
	char command[512] = {0};
	char newImagePath[256] = {0};
	
	// gets a pointer to the extension
	char *dotPtr = strrchr(imagePath,'.');
	
	// creates a new image path
	sprintf(newImagePath, "%stmp_%d%s", tempPath, tmpNum, dotPtr);

	// the image isn't in the trash directory
	if ( strstr(imagePath, "/.stopmotion/trash/") == NULL ) {
		if ( strcmp(imagePath, newImagePath) != 0) {
			// constructs a copy command and executes it
			sprintf(command, "/bin/cp %s %s", imagePath, newImagePath);
			system(command);
		}
	}
	else {
		if ( strcmp(imagePath, newImagePath) != 0) {
			rename(imagePath, newImagePath);
			--trashNum;
		}
	}
	
	delete [] imagePath;
	imagePath = new char[strlen(newImagePath) + 1];
	strcpy(imagePath, newImagePath);

	++tmpNum;
}


void Frame::moveToTrash()
{
	char newImagePath[256] = {0};
	
	char *dotPtr = strrchr(imagePath,'.');
	sprintf(newImagePath, "%strash_%d%s", trashPath, trashNum, dotPtr);

	rename(imagePath, newImagePath);
	
	delete [] imagePath;
	imagePath = new char[strlen(newImagePath) + 1];
	strcpy(imagePath, newImagePath);

	++trashNum;
	--tmpNum;
}


void Frame::markAsProjectFile()
{
	isProjectFile = true;
}


void Frame::playSounds(AudioDriver *driver)
{
	unsigned int numElem = sounds.size();
	if (numElem > 0) {
		for (unsigned int i = 0; i < numElem; i++) {
				driver->addAudioFile(sounds[i]);
		}
		driver->playInThread();
	}
}

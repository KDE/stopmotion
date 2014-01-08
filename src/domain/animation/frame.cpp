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
#include "frame.h"

#include "src/technical/audio/oggvorbis.h"
#include "src/technical/util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <libgen.h>


unsigned int Frame::tmpNum   = 0;
unsigned int Frame::trashNum = 0;
char Frame::tempPath[PATH_MAX]   = {0};
char Frame::trashPath[PATH_MAX]  = {0};


Frame::Frame(const char *filename)
{
	assert(filename != NULL);
	
	snprintf(tempPath, sizeof(tempPath), "%s/.stopmotion/tmp/", getenv("HOME"));
	snprintf(trashPath, sizeof(trashPath), "%s/.stopmotion/trash/", getenv("HOME"));
	
	int len = strlen(filename) + 1;
	imagePath = new char[len];
	strcpy(imagePath, filename);
	isProjectFile = false;
	soundNum = -1;
	
	assert(tempPath  != NULL);
	assert(trashPath != NULL);
	assert(imagePath != NULL);
}


Frame::~Frame()
{
	delete [] imagePath;
	imagePath = NULL;
	
	unsigned int numElem = sounds.size();
	for (unsigned int i = 0; i < numElem; ++i) {
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
	
	// Check if the file exsists, which it probably should do. 
	// The setFilename function will fail if the file doesn't exists,
	// but paranoid people have to dobbel check :-).
	if ( access(filename, F_OK) == 0 ) {
		// Create a new path
		char *imgId = getImageId();
		char newSoundPath[PATH_MAX] = {0};
		snprintf(newSoundPath, sizeof(newSoundPath), "%s%s_snd_%d%s",
			tempPath, imgId, ++soundNum, strrchr(filename,'.'));
		delete [] imgId;
		
		// Check if the sound already is inside the tmp directory.
		// (This can be the fact if we runs in recovery mode.)
		if ( strstr(filename, "/.stopmotion/tmp/") == NULL ) {
			Util::copyFile(newSoundPath, filename);
		}
		else {
			rename(filename, newSoundPath);
		}
		
		// Update with the new path
		f->setFilename(newSoundPath);
		// and add it to the vector
		sounds.push_back(f);

		stringstream ss;
		ss << "Sound" << soundNum;
		soundNames.push_back( ss.str() );
		return 0;
	}
	return 1;
}


void Frame::removeSound(unsigned int soundNumber)
{
	assert(sounds[soundNumber] != NULL);
	
	AudioFormat *f = sounds[soundNumber];
	if ( access( f->getSoundPath(), F_OK ) == 0 ) {
		unlink( f->getSoundPath() );
	}
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


void Frame::moveToProjectDir(
		const char *imageDir, const char *soundDir , unsigned int imgNum )
{
	moveToImageDir(imageDir, imgNum);
	if (sounds.size() > 0) {
		moveToSoundDir(soundDir);
	}
}


void Frame::moveToImageDir(const char *directory, unsigned int imgNum)
{
	assert(directory != 0);
	
	char newPath[PATH_MAX]  = {0};
	char filename[12] = {0};
	char tmp[7] = {0};
	
	snprintf(tmp, 7, "%d", imgNum);
	int fileLength = strlen(tmp);
	
	// creates a filename with six characters as total length,
	// empty characters are filled with zeros
	for (int i = 0; i < 6 - fileLength; ++i) {
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
	
	rename(imagePath, newPath);
	
	delete [] imagePath;
	imagePath = NULL;
	
	imagePath = new char[strlen(newPath) + 1];
	strcpy(imagePath, newPath);
}


void Frame::moveToSoundDir(const char *directory)
{
	assert(directory != NULL);
	
	// Gets the id for this frame
	char *imgId = getImageId();
	
	// Move all of the sounds belonging to this frame
	// to the sounds directory
	unsigned int numSounds = sounds.size();
	for (unsigned int i = 0; i < numSounds; ++i) {
		AudioFormat *f = sounds[i];
		char *soundPath = f->getSoundPath();
		
		// Create a new sound path
		char newSoundPath[PATH_MAX] = {0};
		snprintf(newSoundPath, sizeof(newSoundPath), "%s%s_snd_%d%s",
			directory, imgId, ++soundNum, strrchr(soundPath,'.'));
		
		if (access(soundPath, F_OK) == 0) {
			// Move from old path to new path
			rename(soundPath, newSoundPath);
		}
		// Update with the new path
		f->setFilename(newSoundPath);
		f = NULL;
		soundPath = NULL;
	}
	
	delete [] imgId;
}


void Frame::copyToTemp()
{
	char newImagePath[PATH_MAX] = {0};
	
	// gets a pointer to the extension
	char *dotPtr = strrchr(imagePath,'.');
	
	// creates a new image path
	snprintf(newImagePath, sizeof(newImagePath), "%stmp_%d%s", tempPath, tmpNum, dotPtr);

	// the image isn't in the trash directory
	if (strstr(imagePath, "/.stopmotion/trash/") == NULL) {
		if (strcmp(imagePath, newImagePath) != 0) {
		  Util::copyFile(newImagePath, imagePath);
		}
	}
	else {
		if (strcmp(imagePath, newImagePath) != 0) {
			rename(imagePath, newImagePath);
		}
	}
	
	delete [] imagePath;
	imagePath = NULL;
	
	imagePath = new char[strlen(newImagePath) + 1];
	strcpy(imagePath, newImagePath);

	++tmpNum;
}


void Frame::moveToTrash()
{
	char newImagePath[PATH_MAX] = {0};
	
	char *dotPtr = strrchr(imagePath,'.');
	snprintf(newImagePath, sizeof(newImagePath), "%strash_%d%s", trashPath, trashNum, dotPtr);

	rename(imagePath, newImagePath);
	
	delete [] imagePath;
	imagePath = new char[strlen(newImagePath) + 1];
	strcpy(imagePath, newImagePath);
	
	// Remove all of the sounds added to this frame. This should be
	// added to the undo history in the future.
	unsigned int numSounds = sounds.size();
	for (unsigned int i = 0; i < numSounds; ++i) {
		AudioFormat *f = sounds[i];
		if (access(f->getSoundPath(), F_OK) == 0) {
			unlink( f->getSoundPath() );
		}
		delete sounds[i];
		sounds[i] = NULL;
	}
	sounds.clear();

	++trashNum;
}


void Frame::markAsProjectFile()
{
	isProjectFile = true;
}


void Frame::playSounds(AudioDriver *driver)
{
	unsigned int numElem = sounds.size();
	if (numElem > 0) {
		for (unsigned int i = 0; i < numElem; ++i) {
				driver->addAudioFile(sounds[i]);
		}
		driver->playInThread();
	}
}


char* Frame::getImageId()
{
	char tmp[PATH_MAX] = {0};
	strcpy(tmp, imagePath);
	char *bname = basename(tmp);
	char *dotPtr = strrchr(imagePath, '.');
	
	char imgID[PATH_MAX] = {0};
	strncpy(imgID, bname, strlen(bname) - strlen(dotPtr) );
	strcat(imgID, "\0");
	
	char *ret = new char[strlen(imgID) + 1];
	strcpy(ret, imgID);
	
	return ret;
}


bool Frame::isProjectFrame()
{
	return isProjectFile;
}


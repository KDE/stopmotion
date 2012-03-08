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
#include "projectserializer.h"

#include "src/foundation/logger.h"
#include "packer.h"
#include "src/domain/animation/frame.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <stdlib.h>
#include <cstring>

const char ProjectSerializer::imageDirectory[] = "images/";
const char ProjectSerializer::soundDirectory[] = "sounds/";


ProjectSerializer::ProjectSerializer()
{
	doc      = NULL;
	dtd      = NULL;
	rootNode = NULL;
	scenes   = NULL;
	images   = NULL;
	sounds   = NULL;
	
	LIBXML_TEST_VERSION;
	
	projectPath = NULL;
	projectFile = NULL;
	imagePath   = NULL;
	soundPath   = NULL;
	xmlFile     = NULL;
	prevProPath = NULL;
	prevImgPath = NULL;
	prevXmlFile = NULL;
}


ProjectSerializer::~ProjectSerializer()
{
	cleanup();
}


const vector<Scene*> ProjectSerializer::open(const char *filename)
{
	assert(filename != NULL);

	bool isNewProFile = setProjectFile(filename);
	char rootDir[PATH_MAX] = {0};
	snprintf(rootDir, sizeof(rootDir), "%s/.stopmotion/packer/", getenv("HOME"));
	char *unpackedAs = unpack(projectFile, rootDir);
	
	if ( isNewProFile ) {
		setProjectPaths(unpackedAs, false);
	}
	
	doc = xmlReadFile(xmlFile, NULL, 0);
	if (doc == NULL) {
		Logger::get().logWarning("Couldn't load XML file");
	}
	
	vector<Scene*> sVect;
	rootNode = xmlDocGetRootElement(doc);
	getAttributes(rootNode, sVect);
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	doc      = NULL;
	dtd      = NULL;
	rootNode = NULL;
	scenes   = NULL;
	images   = NULL;
	sounds   = NULL;
	free(unpackedAs);
	unpackedAs = NULL;
	
	return sVect;
}


// check if the user wants to save an opened project to an another file.
bool ProjectSerializer::save( 	const char *filename, 
								const vector<Scene*>& sVect, 
								Frontend *frontend	)
{
	assert(filename != NULL);
	assert(frontend != NULL);
	
	if ( setProjectFile(filename) ) {
		setProjectPaths(projectPath, true);
	}
	// Only for backwards compability
	else if (access(soundPath, F_OK) != 0) {
		mkdir(soundPath, 0755);
	} 
	
	doc = xmlNewDoc(BAD_CAST "1.0");
	dtd = xmlCreateIntSubset(doc, BAD_CAST "smil", BAD_CAST "-//W3C//DTD SMIL 2.0//EN",
				BAD_CAST"http://www.w3.org/2001/SMIL20/SMIL20.dtd");
	
	rootNode = xmlNewNode(NULL, BAD_CAST "smil");
	xmlNewProp(rootNode, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2001/SMIL20/Language");
	xmlNewProp(rootNode, BAD_CAST "xml:lang", BAD_CAST "en");
	xmlNewProp(rootNode, BAD_CAST "title", BAD_CAST "Stopmotion");
	xmlDocSetRootElement(doc, rootNode);
	
	setAttributes(sVect, frontend);
	
	bool isSaved = saveDOMToFile(doc);

	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	doc      = NULL;
	dtd      = NULL;
	rootNode = NULL;
	scenes   = NULL;
	images   = NULL;
	sounds   = NULL;
	
	pack(projectPath, projectFile);
	cleanupPrev();
	
	unsigned int numElem = sVect.size();
	frontend->updateProgress(numElem);
	frontend->hideProgress();
	
	return isSaved;
}


const char* ProjectSerializer::getProjectFile()
{
	return projectFile;
}


void ProjectSerializer::setAttributes(const vector<Scene*>& sVect, Frontend *frontend)
{
	xmlNodePtr node = NULL;
	char *absPath    = NULL;
	Frame *frame     = NULL;
	AudioFormat *sound = NULL;
	unsigned int index = 0;
	
	// Removes frames which already are saved. Had to do this to prevent
	// frames to overwrite each other.
	unsigned int numScenes = sVect.size();
	for (unsigned int m = 0; m < numScenes; ++m) {
		vector<Frame*> frames = sVect[m]->getFrames();
		
		unsigned int numFrames = frames.size();
		for (unsigned int l = 0; l < numFrames; ++l) {
			frame = frames[l];
			if ( frame->isProjectFrame() ) {
				frame->copyToTemp();
			}
		}
	}
	
	scenes = xmlNewChild(rootNode, NULL, BAD_CAST "scenes", NULL);
	
	//unsigned int numScenes = sVect.size();
	frontend->showProgress("Saving scenes to disk ...", numScenes);
	for (unsigned int i = 0; i < numScenes; ++i) {
		frontend->updateProgress(i);
		// Scenes
		node = xmlNewChild(scenes, NULL, BAD_CAST "seq", NULL);
		
		// Images
		images = xmlNewChild(node, NULL, BAD_CAST "images", NULL);
		vector<Frame*> frames = sVect[i]->getFrames();
		unsigned int numFrames = frames.size();
		for (unsigned int j = 0; j < numFrames; ++j) {
			frame = frames[j];
			frame->moveToProjectDir(imagePath, soundPath, ++index);
			frame->markAsProjectFile();
			absPath = frame->getImagePath();
			char *relPath = basename(absPath);
			node = xmlNewChild(images, NULL, BAD_CAST "img", NULL);
			xmlNewProp(node, BAD_CAST "src", BAD_CAST relPath);
			
			// Sounds
			unsigned int numSounds = frame->getNumberOfSounds();
			if (numSounds > 0) {
				sounds = xmlNewChild(node, NULL, BAD_CAST "sounds", NULL);
				vector<AudioFormat*> audioFiles = frame->getSounds();
				for (unsigned int k = 0; k < numSounds; ++k) {
					sound = audioFiles[k];
					relPath = basename(audioFiles[k]->getSoundPath());
					node = xmlNewChild(sounds, NULL, BAD_CAST "audio", NULL);
					xmlNewProp(node, BAD_CAST "src", BAD_CAST relPath);
					xmlNewProp(node, BAD_CAST "alt", BAD_CAST frame->getSoundName(i));
				}
			}
		}
		frames.clear();
	}
}


void ProjectSerializer::getAttributes(xmlNodePtr node, vector<Scene*>& sVect)
{
	xmlNodePtr currNode = NULL;
	for (currNode = node; currNode; currNode = currNode->next) {
		if (currNode->type == XML_ELEMENT_NODE) {
			char *nodeName = (char*)currNode->name;
			// We either have a image node or a sound node
			if ( strcmp(nodeName, "img") == 0 || strcmp(nodeName, "audio") == 0 ) {
				char *filename = (char*)xmlGetProp(currNode, BAD_CAST "src");
				if (filename != NULL) {
					char tmp[PATH_MAX] = {0};
					// The node is a image node
					if ( strcmp(nodeName, "img") == 0 ) {
						snprintf(tmp, sizeof(tmp), "%s%s", imagePath, filename);
						Frame *f = new Frame(tmp);
						Scene *s = sVect.back();
						s->addSavedFrame(f);
						f->markAsProjectFile();
					}
					// The node is a sound node
					else {
						snprintf(tmp, sizeof(tmp), "%s%s", soundPath, filename);
						Scene *s = sVect.back();
						Frame *f = s->getFrame(s->getSize() - 1);
						f->addSound(tmp);
						char *soundName = (char*)xmlGetProp(currNode, BAD_CAST "alt");
						if (soundName != NULL) {
							unsigned int soundNum = f->getNumberOfSounds() - 1;
							f->setSoundName(soundNum,  soundName);
							xmlFree((xmlChar*)soundName);
						}
					}
					xmlFree((xmlChar*)filename);
				}
			}
			// The node is a scene node
			else if ( strcmp((char*)currNode->name, "seq") == 0 ) {
				Scene *s = new Scene();
				sVect.push_back(s);
			}
		}
		getAttributes(currNode->children, sVect);
	}
}


bool ProjectSerializer::saveDOMToFile(xmlDocPtr doc)
{
	int ret = xmlSaveFormatFile(xmlFile, doc, 1);
	if (ret == -1) {
		Logger::get().logWarning("Couldnt save DOM to file");
		return false;
	}
	return true;
}


void ProjectSerializer::setProjectPaths(const char *unpacked, bool isSave)
{
	if (isSave) {
		storeOldPaths();
	}
	
	char bigTmp[PATH_MAX] = {0};
	char tmp[PATH_MAX] = {0};

	if (isSave) {
		char newDir[PATH_MAX] = {0};
		strcpy(tmp, projectFile);
		char *bname = basename(tmp);
		char *dotPtr = strrchr(bname, '.');
		strncpy( newDir, bname, strlen(bname) - strlen(dotPtr) );
		snprintf(bigTmp, sizeof(bigTmp), "%s/.stopmotion/packer/%s/", getenv("HOME"), newDir);
	}
	else {
		snprintf(bigTmp, sizeof(bigTmp), "%s", unpacked);
	}
	
	projectPath = new char[strlen(bigTmp) + 1];
	strcpy(projectPath, bigTmp);
		
	strncpy(bigTmp, projectPath, strlen(projectPath) - 1 );
	strcat(bigTmp, "\0");
	char *bname= basename(bigTmp);
	strcpy(tmp, bname);
	
	snprintf(bigTmp, sizeof(bigTmp), "%s%s.dat", projectPath, tmp);
	xmlFile = new char[strlen(bigTmp) + 1];
	strcpy(xmlFile, bigTmp);
		
	snprintf(bigTmp, sizeof(bigTmp), "%s%s", projectPath, imageDirectory);
	imagePath = new char[strlen(bigTmp) + 1];
	strcpy(imagePath, bigTmp);
	
	snprintf(bigTmp, sizeof(bigTmp), "%s%s", projectPath, soundDirectory);
	soundPath = new char[strlen(bigTmp) + 1];
	strcpy(soundPath, bigTmp);

	if (isSave) {
		if ( access(projectPath, F_OK) != 0 ) {
			mkdir(projectPath, 0755);
			mkdir(imagePath, 0755);
			mkdir(soundPath, 0755);
		}
	}
}


bool ProjectSerializer::setProjectFile(const char *filename)
{
	assert(filename != NULL);
	if (projectFile == NULL || strcmp(projectFile, filename) != 0) {
		char tmp[PATH_MAX] = {0};
		strcpy(tmp, filename);
		char *dotPtr = strrchr(tmp, '.');
	
		// if the file doesn't have an extension, or if it has and 
		// this one isn't '.sto'
		if (dotPtr == NULL || strcmp(dotPtr, ".sto") != 0 ) {
			strcat(tmp, ".sto");
		}
	
		if (projectFile != NULL) {
			delete [] projectFile;
			projectFile = NULL;
		}
		
		projectFile = new char[strlen(tmp) + 1];
		strcpy(projectFile, tmp);
		return true;
	}
	return false;
}


const char* ProjectSerializer::getProjectPath()
{
	return projectPath;
}


const char* ProjectSerializer::getImagePath()
{
	return imagePath;
}


void ProjectSerializer::cleanup()
{
	if (projectPath != NULL) {
		char command[PATH_MAX] = {0};
		snprintf(command, PATH_MAX, "rm -rf \"%s\"", projectPath);
		system(command);
		
		delete [] projectPath;
		projectPath = NULL;
		delete [] projectFile;
		projectFile = NULL;
		delete [] imagePath;
		imagePath = NULL;
		delete [] soundPath;
		soundPath = NULL;
		delete [] xmlFile;
		xmlFile = NULL;
	
		cleanupPrev();
	}
}


void ProjectSerializer::cleanupPrev()
{
	if (prevProPath != NULL) {
		char command[PATH_MAX] = {0};
		snprintf(command, sizeof(command), "rm -rf \"%s\"", prevProPath);
		
		delete [] prevProPath;
		prevProPath = NULL;
		delete [] prevImgPath;
		prevImgPath = NULL;
		delete [] prevXmlFile;
		prevXmlFile = NULL;
	}
}


void ProjectSerializer::storeOldPaths()
{
	if (projectPath != NULL) {
		prevProPath = new char[strlen(projectPath) + 1];
		strcpy(prevProPath, projectPath);
		prevImgPath = new char[strlen(imagePath) + 1];
		strcpy(prevImgPath, imagePath);
		prevXmlFile = new char[strlen(xmlFile) + 1];
		strcpy(prevXmlFile, xmlFile);
	
		delete [] projectPath;
		projectPath = NULL;
		delete [] imagePath;
		imagePath = NULL;
		delete [] xmlFile;
		xmlFile = NULL; 
	}
}


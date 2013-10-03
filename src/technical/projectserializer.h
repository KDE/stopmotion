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
#ifndef PROJECTSERIALIZER_H
#define PROJECTSERIALIZER_H

#include "src/domain/animation/scene.h"
#include "src/presentation/frontends/frontend.h"

#include <libxml/tree.h>


/**
 * Class for serializing the project. It creates differents directories containing
 * the files belonging to the current project. Finally everything is packed in
 * a tarball.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad.
 */
class ProjectSerializer
{
public:
	ProjectSerializer();
	~ProjectSerializer();
	
	/**
	 * Creates necessary project paths and opens the project file.
	 * @param filename the project file
	 * @return a vector containing the scenes stored in the project file
	 */
	const vector<Scene*> open(const char *filename);
	
	/**
	 * Saves the files in differents directories and packs the resulting files
	 * in a tarball with the same name as 'filename' plus the '.sto' extension.
	 * @param filename the project file to store the files within
	 * @param scenes the scenes to be saved
	 * @param frontend the frontend to display progress to
	 * @return true if saving was successfull, false otherwise
	 */
	bool save(const char *filename, const vector<const Scene*>& scenes,
			Frontend *frontend);
	
	/**
	 * Retrieves the project file.
	 * @return the project path if it exist, NULL otherwise
	 */
	const char* getProjectFile();
	
	/**
	 * Retrieves the absolute project path. This is the path where the files
	 * within the tarball are located. (~/.stopmotion/packer/xxx)
	 * @return the project path if it exist, NULL otherwise
	 */
	const char* getProjectPath();
	
	/**
	 * Retrieves the absolute image path. This is the path where the images
	 * can be found. (~/.stopmotion/packer/xxx/images)
	 * @return 
	 */
	const char* getImagePath();
	
	/**
	 * Deletes unessecary files and pointers.
	 */
	void cleanup();
	
private:
	xmlDocPtr doc;
	xmlDtdPtr dtd;
	xmlNodePtr rootNode;
	xmlNodePtr scenes;
	xmlNodePtr images;
	xmlNodePtr sounds;
	
	static const char imageDirectory[]; // relative to projectDirectory
	static const char soundDirectory[]; // releative to projectDirectory
	char *projectPath;// absolute
	char *projectFile;// absolute
	char *imagePath;  // absolute
	char *soundPath;  // absolute
	char *xmlFile;    // absolute
	char *prevProPath;// absolute
	char *prevImgPath;// absolute
	char *prevXmlFile;// absolute
	
	void setAttributes(const vector<const Scene*>& scenes, Frontend *frontend);
	void getAttributes(xmlNodePtr node, vector<Scene*>& scenes);
	bool saveDOMToFile(xmlDocPtr doc);
	void setProjectPaths(const char *unpacked, bool isSave);
	bool setProjectFile(const char *filename);
	void cleanupPrev();
	void storeOldPaths();
};

#endif

/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

#include <libxml/tree.h>

#include <vector>

class AnimationImpl;
class Scene;
class Frontend;

/**
 * Class for serializing the project.
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
	const std::vector<Scene*> open(const char *filename);

	/**
	 * Saves the files in a tarball with the name {@a filename} plus the
	 * {@c .sto} extension.
	 * @param filename the project file to store the files within
	 * @param scenes the scenes to be saved
	 * @param frontend the frontend to display progress to
	 */
	void save(const char *filename, const AnimationImpl& scenes,
			Frontend *frontend);

	/**
	 * Retrieves the project file.
	 * @return the project path if it exist, NULL otherwise
	 */
	const char* getProjectFile();

private:
	char* projectFile;
	void setAttributes(xmlNodePtr rootNode,
			const AnimationImpl& anim, Frontend *frontend);
	void getAttributes(xmlNodePtr node, std::vector<Scene*>& scenes);
	void saveDOMToFile(xmlDocPtr doc, const char* filename);
	void setProjectFile(const char *filename);
};

#endif

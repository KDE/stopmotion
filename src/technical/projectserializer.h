/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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

#include <exception>
#include <vector>

class AnimationImpl;
class Scene;
class Frontend;

class FileException : public std::exception {
	char buffer[100];
public:
	FileException(const char* functionName, int errorno);
	const char* what() const throw();
};

/**
 * Class for serializing the project.
 */
class ProjectSerializer {
public:
	ProjectSerializer();
	~ProjectSerializer();

	/**
	 * Opens the project's model ({@c .dat}) file.
	 * @param [out] out A vector to be filled with the scenes stored in the
	 * project file, if the function is successful.
	 * @param filename The model file.
	 * @return {@c true} if successful, {@c false} otherwise.
	 */
	static bool openDat(std::vector<Scene*>& out, const char* filename);

	/**
	 * Creates necessary project paths and opens the project ({@c .sto}) file.
	 * @param filename The project file
	 * @return A vector containing the scenes stored in the project file.
	 */
	std::vector<Scene*> openSto(const char *filename);

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
	 * @return the project path if it exist, NULL otherwise. Ownership is not
	 * returned.
	 */
	const char* getProjectFile();

	/**
	 * After this call (and until a subsequent call to {@ref save} or
	 * {@ref openSto}) {@ref getProjectFile} will return a null pointer.
	 */
	void resetProjectFile();

	/**
	 * After this call (and until a subsequent call to {@ref save} or
	 * {@ref openSto}) {@ref getProjectFile} will return a string equal to
	 * {@a filename}.
	 * @param filename The new file name to set. Ownership is not passed. A
	 * copy is taken, so the contents of the string need not be preserved. If
	 * null is passed, {@ref getProjectFile} will subsequently return null.
	 */
	void resetProjectFile(const char* filename);

private:
	char* projectFile;
	void setAttributes(xmlNodePtr rootNode,
			const AnimationImpl& anim, Frontend *frontend);
	static void getAttributes(xmlNodePtr node, std::vector<Scene*>& scenes);
	void saveDOMToFile(xmlDocPtr doc, const char* filename);
	void setProjectFile(const char *filename);
};

#endif

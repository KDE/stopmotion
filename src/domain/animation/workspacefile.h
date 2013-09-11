/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

#ifndef WORKSPACEFILE_H_
#define WORKSPACEFILE_H_

class TemporaryWorkspaceFile;

class WorkspaceFile {
	char* fullPath;
	const char* namePart;
	WorkspaceFile(const WorkspaceFile&);
	WorkspaceFile& operator=(const WorkspaceFile);
public:
	/**
	 * Sets filename as the owned file.
	 * @note The file is not kept open by this class.
	 * @param filename The file name (just name and extension, without path)
	 * of the file, which must be in the workspace directory
	 * ({@c ~/.stopmotion/tmp})
	 */
	WorkspaceFile(const char* filename);
	/**
	 * Takes ownership of the file owned by @c t, thereby preventing it from
	 * being deleted when @c t is destroyed. @c t is emptied by this operation
	 * and so cannot be used again.
	 */
	WorkspaceFile(TemporaryWorkspaceFile t);
	~WorkspaceFile();
	/**
	 * Takes ownership of the file owned by @c t, thereby preventing it from
	 * being deleted when @c t is destroyed. @c t is emptied by this operation
	 * and so cannot be used again.
	 */
	WorkspaceFile& operator=(TemporaryWorkspaceFile t);
	/**
	 * Gets the file's basename.
	 * @return The file's name (i.e. with no directory part but including any
	 * extension).
	 */
	const char* basename() const;
	/**
	 * Gets the file's full path.
	 * @return the file's full path, which will be in the workspace directory
	 * (i.e. @c ~/.stopmotion/tmp but specified relative to /, not ~).
	 */
	const char* path() const;
	/**
	 * Clears (creating if necessary) the workspace directory
	 */
	static void clear();
};

class TemporaryWorkspaceFile {
	char* path;
	const char* namePart;
	bool toBeDeleted;
	TemporaryWorkspaceFile(const TemporaryWorkspaceFile&);
	TemporaryWorkspaceFile& operator=(const TemporaryWorkspaceFile);
	friend class WorkspaceFile;
public:
	/**
	 * Copy file with path @c filename into the workspace directory
	 * (@c ~/.stopmotion/tmp ) unless it is already in this directory.
	 * A freshly-copied file will be deleted on destruction unless a
	 * @ref WorkspaceFile is constructed from it beforehand.
	 * @note The file is not kept open by this class.
	 * @param filename The full path to the file.
	 */
	TemporaryWorkspaceFile(const char* filename);
	~TemporaryWorkspaceFile();
};

#endif /* WORKSPACEFILE_H_ */

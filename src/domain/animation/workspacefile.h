/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef WORKSPACEFILE_H_
#define WORKSPACEFILE_H_

#include <stdint.h>
#include <exception>

class TemporaryWorkspaceFile;

class CopyFailedException : public std::exception {
	const char* what() const _GLIBCXX_USE_NOEXCEPT;
};

class WorkspaceDirectoryCreationException : public std::exception {
	const char* what() const _GLIBCXX_USE_NOEXCEPT;
};

/**
 * Represents the filename of a file in the workspace (~/.stopmotion/tmp/).
 * The file is not held open, nor is it deleted on destruction.
 */
class WorkspaceFile {
	char* fullPath;
	const char* namePart;
	WorkspaceFile(const WorkspaceFile&);
	WorkspaceFile& operator=(const WorkspaceFile);
public:
	enum FreshFilename {
		freshFilename
	};
	/**
	 * Creates a WorkspaceFile referring to no file. Both @ref basename and
	 * @ref path will return null until a @ref TemporaryWorkspaceFile is
	 * assigned to it.
	 */
	WorkspaceFile();
	/**
	 * Creates a fresh filename without a file corresponding to it.
	 * @param extension Characters that must terminate the name, for
	 * example ".jpg".
	 */
	WorkspaceFile(const char* extension, FreshFilename);
	/**
	 * Creates a WorkspaceFile referring to the same file as {@a t}.
	 * @param t The file to set it to.
	 */
	WorkspaceFile(TemporaryWorkspaceFile& t);
	~WorkspaceFile();
	/**
	 * Takes ownership of the file owned by @a t, thereby preventing it from
	 * being deleted when @a t is destroyed. @a t is emptied by this operation
	 * and so cannot be used again. This operation will not fail.
	 */
	void take(TemporaryWorkspaceFile& t);
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
	 * Swaps the content of this object with another.
	 * @param w The object to swap contents with.
	 */
	void swap(WorkspaceFile& w);
	/**
	 * Clears (creating if necessary) the workspace directory
	 */
	static void clear();
	/**
	 * Returns the current sound number counter. This is cleared by a call to
	 * {@ref clear}. Remember to call {@ref nextSoundNumber} if the sound
	 * number actually gets used.
	 * @return One more than the number of calls to {@ref nextSoundNumber}
	 * since the last call to {@ref clear}.
	 */
	static uint32_t getSoundNumber();
	/**
	 * Increments the sound counter.
	 */
	static void nextSoundNumber();
};

/**
 * Represents the filename of a newly-created file in the workspace
 * (~/.stopmotion/tmp/). This file will be deleted upon destruction unless it
 * has been assigned to a @ref WorkspaceFile beforehand.
 */
class TemporaryWorkspaceFile {
	char* fullPath;
	const char* namePart;
	bool toBeDeleted;
	TemporaryWorkspaceFile(const TemporaryWorkspaceFile&);
	TemporaryWorkspaceFile& operator=(const TemporaryWorkspaceFile);
	friend class WorkspaceFile;
	/**
	 * @throws CopyFailedException if the copy failed.
	 */
	void copyToWorkspace(const char* filename);
public:
	enum ForceCopy {
		forceCopy
	};
	enum AlreadyAWorkspaceFile {
		alreadyAWorkspaceFile
	};
	/**
	 * Copy file with path @a filename into the workspace directory
	 * ({@c ~/.stopmotion/tmp}) unless it is already in this directory.
	 * A freshly-copied file will be deleted on destruction unless a
	 * @ref WorkspaceFile is constructed from it beforehand.
	 * @note The file is not kept open by this class.
	 * @param filename The full path to the file.
	 * @throws CopyFailedException if the copy failed.
	 */
	TemporaryWorkspaceFile(const char* filename);
	/**
	 * Copy file with path @a filename into the workspace directory,
	 * even if it is already in this directory. The file will be deleted on
	 * destruction unless it has been assigned to a @ref WorkspaceFile
	 * beforehand.
	 * @note The file is not kept open by this class.
	 * @param filename The full path to the file. Ownership is not passed.
	 * @throws CopyFailedException if the copy failed.
	 */
	TemporaryWorkspaceFile(const char* filename, ForceCopy);
	/**
	 * Refers to a file already in the workspace. The file counter will be
	 * advanced if necessary to avoid new files from clashing with this one.
	 * This file will not be deleted on destruction.
	 * @param basename the file name (including extension) relative to the
	 * workspace directory. Ownership is not passed.
	 */
	TemporaryWorkspaceFile(const char* basename, AlreadyAWorkspaceFile);
	~TemporaryWorkspaceFile();
	/**
	 * Prevents the file from being deleted on destruction.
	 */
	void retainFile() {
		toBeDeleted = false;
	}
	/**
	 * Returns the path of the file.
	 * @return Ownership is not returned.
	 */
	const char* path() {
		return fullPath;
	}
	/**
	 * Returns the basename (with extension) of the file.
	 * @return Ownership is not returned.
	 */
	const char* basename() {
		return namePart;
	}
};

#endif /* WORKSPACEFILE_H_ */

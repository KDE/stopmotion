/***************************************************************************
 *   Copyright (C) 2013-2017 by Linuxstopmotion contributors;              *
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

/**
 * Represents the filename of a file in the workspace (~/.stopmotion/).
 * The file is not held open, nor is it deleted on destruction.
 * @par
 * To be used by commands in the undo system and objects manipulated by such
 * commands.
 */
class WorkspaceFile {
	char* fullPath;
	const char* namePart;
	void setFilename(const char* basename, bool inFrames = false);
public:
	enum NewModelFile { newModelFile };
	enum CurrentModelFile { currentModelFile };
	enum CommandLogFile { commandLogFile };
	enum CapturedImage { capturedImage };
	enum PreferencesFile { preferencesFile };
	enum PreferencesFileOld { preferencesFileOld };
	enum AndClear { doNotClear, andClear };
	WorkspaceFile(const WorkspaceFile&);
	WorkspaceFile& operator=(const WorkspaceFile&);
	/**
	 * Creates a WorkspaceFile referring to no file. Both @ref basename and
	 * @ref path will return null until a @ref TemporaryWorkspaceFile is
	 * assigned to it.
	 */
	WorkspaceFile();
	/**
	 * Creates a WorkspaceFile referring to a sound or image file already in
	 * the workspace (having been created by {@c TemporaryWorkspaceFile}).
	 * @param name The basename of the file (i.e. the return value of
	 * {@ref TemporaryWorkspaceFile::basename}).
	 */
	WorkspaceFile(const char* name);
	/**
	 * Refers to the "new" model file, which should be used by the recovery
	 * system if either the "current" model file or the command log cannot be
	 * found. The command log should never be used in conjunction with this
	 * file. This should only be necessary if a crash occurred during project
	 * save.
	 */
	WorkspaceFile(NewModelFile);
	/**
	 * Refers to the "current" model file. If it and the command log are
	 * present, the recovery system restores from this and then plays back the
	 * commands from the command log.
	 */
	WorkspaceFile(CurrentModelFile);
	/**
	 * Refers to the command log. This holds the commands that were issued
	 * since the "current" model file was written.
	 */
	WorkspaceFile(CommandLogFile);
	/**
	 * Refers to the image written to by the capture command.
	 */
	WorkspaceFile(CapturedImage);
	/**
	 * Refers to the preferences file.
	 */
	WorkspaceFile(PreferencesFile);
	/**
	 * Refers to the backup preferences file.
	 */
	WorkspaceFile(PreferencesFileOld);
	~WorkspaceFile();
	/**
	 * Gets the file's basename.
	 * @return The file's name (i.e. with no directory part but including any
	 * extension).
	 */
	const char* basename() const;
	/**
	 * Gets the file's full path.
	 * @return the file's full path, which will be in the workspace directory
	 * (i.e. @c ~/.stopmotion but specified relative to /, not ~).
	 */
	const char* path() const;
	/**
	 * Swaps the content of this object with another.
	 * @param w The object to swap contents with.
	 */
	void swap(WorkspaceFile& w);
	/**
	 * Ensures that the ~/.stopmotion and frames directories exist if they do
	 * not already.
	 * @param clear Pass @c andClear to clear the frames directory contents, if
	 * any.
	 */
	static void ensureStopmotionDirectoriesExist(AndClear clear = doNotClear);
	/**
	 * Clears (creating if necessary) model files, frames, sounds and logs from
	 * the workspace directory
	 */
	static void clear();
	/**
	 * Returns the current sound number counter. This is cleared by a call to
	 * {@ref clear}. Remember to call {@ref nextSoundNumber} if the sound
	 * number actually gets used. This is the number to be used in the default
	 * readable name of a newly-added sound.
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
 * Represents a type of file that can be copied into the workspace.
 */
class WorkspaceFileType {
public:
	virtual ~WorkspaceFileType() = 0;
	/**
	 * Returns the preferred file name extension, including any dot.
	 * @param path The original file path.
	 * @return The extension, ownership is not returned.
	 */
	virtual const char* preferredExtension(const char* path) const = 0;
	/**
	 * Determines if the argument is (probably?) this type of file.
	 * @param path The file path.
	 * @return true if and only if this is a file that might be used
	 * for this sort of file.
	 */
	virtual bool isType(const char* path) const = 0;
	/**
	 * Returns a representation of image files.
	 * @return The type, ownership is not returned.
	 */
	static const WorkspaceFileType& image();
	/**
	 * Returns a representation of sound files.
	 * @return The type, ownership is not returned.
	 */
	static const WorkspaceFileType& sound();
};

/**
 * Represents the filename of a newly-created file in the workspace
 * (~/.stopmotion/). This file will be deleted upon destruction unless it
 * has been assigned to a @ref WorkspaceFile beforehand.
 * @par
 * To be used by the facade in front of the undo system.
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
	void copyToWorkspace(const char* filename, const WorkspaceFileType& type);
public:
	enum ForceCopy {
		forceCopy
	};
	/**
	 * Copy file with path @a filename into the workspace directory
	 * ({@c ~/.stopmotion}) unless it is already in this directory.
	 * A freshly-copied file will be deleted on destruction unless
	 * @p retainFile has been called.
	 * @note The file is not kept open by this class.
	 * @param filename The full path to the file.
	 * @param type The type of file to be copied.
	 * @throws UiException with code failedToCopyFilesToWorkspace if
	 * the copy failed.
	 * @throws UiException with code unsupportedImageType if an image
	 * type that is not supported is passed.
	 */
	TemporaryWorkspaceFile(const char* filename, const WorkspaceFileType& type);
	/**
	 * Copy file with path @a filename into the workspace directory,
	 * even if it is already in this directory. The file will be deleted on
	 * destruction unless @p retainFile has been called.
	 * @note The file is not kept open by this class.
	 * @param filename The full path to the file. Ownership is not passed.
	 * @param type The type of file to be copied.
	 * @throws UiException with code failedToCopyFilesToWorkspace if
	 * the copy failed.
	 * @throws UiException with code unsupportedImageType if an image
	 * type that is not supported is passed.
	 */
	TemporaryWorkspaceFile(const char* filename, const WorkspaceFileType& type,
			ForceCopy);
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
	const char* path() const {
		return fullPath;
	}
	/**
	 * Returns the basename (with extension) of the file.
	 * @return Ownership is not returned.
	 */
	const char* basename() const {
		return namePart;
	}
};

class ExportDirectory {
	char* p;
public:
	ExportDirectory();
	~ExportDirectory();
	const char* path() const {
		return p;
	}
	void makeEmpty();
};

#endif /* WORKSPACEFILE_H_ */

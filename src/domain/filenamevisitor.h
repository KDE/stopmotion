/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILENAMEVISITOR_H_
#define FILENAMEVISITOR_H_

/**
 * Interface for visitors that want to find out which files are referenced by a
 * command, frame, scene, animation or undo history.
 */
class FileNameVisitor {
protected:
	virtual ~FileNameVisitor();
public:
	/** Called to report an image file referenced. */
	virtual void visitImage(const char*) = 0;
	/** Called to report a sound file referenced. */
	virtual void visitSound(const char*) = 0;
	/** For an animation, reports when a scene starts. */
	virtual void reportNewScene();
};

#endif /* FILENAMEVISITOR_H_ */

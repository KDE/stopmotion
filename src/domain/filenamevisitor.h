/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
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
};

#endif /* FILENAMEVISITOR_H_ */

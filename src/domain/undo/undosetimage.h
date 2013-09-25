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


#ifndef UNDOSETIMAGE_H_
#define UNDOSETIMAGE_H_

#include "command.h"
#include "workspacefile.h"

class SceneVector;

class UndoSetImage : public Command {
	SceneVector& sv;
	int32_t sc;
	int32_t fr;
	WorkspaceFile image;
public:
	UndoSetImage(SceneVector& model, int32_t scene, int32_t frame,
			TemporaryWorkspaceFile& w);
	~UndoSetImage();
	Command* execute();
};

class UndoSetImageFactory : public CommandFactory {
	SceneVector& sv;
public:
	UndoSetImageFactory(SceneVector& model);
	~UndoSetImageFactory();
	Command* create(Parameters& ps);
};

#endif /* UNDOSETIMAGE_H_ */

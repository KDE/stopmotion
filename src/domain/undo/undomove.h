/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors.                   *
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
#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include "command.h"

class SceneVector;

class UndoMove : public Command {
	SceneVector& sv;
	int32_t fromSc;
	int32_t fromFr;
	int32_t frameCount;
	int32_t toSc;
	int32_t toFr;
public:
	UndoMove(SceneVector& model, int fromScene, int fromFrame, int count,
			int toScene, int toFrame);
	~UndoMove();
	Command* execute();
};

class UndoMoveFactory : public CommandFactory {
	SceneVector& sv;
public:
	UndoMoveFactory(SceneVector& model);
	~UndoMoveFactory();
	Command* create(Parameters& ps);
};

#endif

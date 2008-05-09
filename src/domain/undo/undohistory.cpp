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
#include "undohistory.h"


UndoHistory::UndoHistory()
{
	this->currentUndo = 0;
}


UndoHistory::~UndoHistory()
{
	unsigned int numElem = undos.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete undos[i];
		undos[i] = NULL;
	}
	this->currentUndo = 0;
}


void UndoHistory::addUndo (Undo *undo) 
{
	if ( currentUndo < undos.size() ) {
		unsigned int numElem = undos.size();
		for (unsigned int i=currentUndo; i < numElem; ++i) {
			delete undos[currentUndo];
			undos.erase(undos.begin() + currentUndo);
		}
	}
	
	if ((currentUndo + 1) > MAX_UNDOES) {
		delete undos.front();
		undos.erase(undos.begin());
		--currentUndo;
	}
	
	this->undos.push_back(undo);
	++currentUndo;
}


bool UndoHistory::undo(AnimationModel *a)
{
	if (currentUndo > 0) {
		undos[--currentUndo]->undo(a);
	}

	if (currentUndo == 0) {
		return false;
	}
	return true;
}


bool UndoHistory::redo(AnimationModel *a)
{
	if (currentUndo < undos.size() && undos.size() > 0) {
		undos[currentUndo++]->redo(a);
	}
	
	if ( currentUndo == undos.size() ) {
		return false;
	}
	return true;
}


void UndoHistory::clear()
{
	unsigned int numElem = undos.size();
	for (unsigned int i = 0; i < numElem; ++i) {
		delete undos[i];
		undos[i] = NULL;
	}
	
	undos.clear();
	this->currentUndo = 0;
}

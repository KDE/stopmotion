/*
 * SPDX-FileCopyrightText: 2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef UNDOREDOOBSERVER_H_
#define UNDOREDOOBSERVER_H_

class UndoRedoObserver {
protected:
	virtual ~UndoRedoObserver();
public:
	virtual void updateCanUndo(bool newCanUndo) = 0;
	virtual void updateCanRedo(bool newCanRedo) = 0;
};

#endif

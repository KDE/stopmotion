/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef EDITMENUHANDLER_H
#define EDITMENUHANDLER_H

#include <QObject>

class FrameBar;
class QStatusBar;
class QWidget;

/**
 * This class handles request related to the editmenu (undo, redo, copy, cut, etc)
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class EditMenuHandler : public QObject
{
	Q_OBJECT
public:
	/**
	 * Creates and initializes the Soundhandler.
	 * @param parent the parent of the soundhandler.
	 * @param sb a statusbar for displaying status information to the user.
	 * @param frameBar the frameBar containing thumbnails
	 * @param name the name of the soundhandler
	 */
	EditMenuHandler( QObject *parent = nullptr, QStatusBar *sb = nullptr, FrameBar *frameBar = nullptr,
			const char *name = nullptr );
	
	
	/**
	 * Sets the gotoMenu property so that it can be closed.
	 * @param gotoMenu the gotoMenu.
	 */
	void setGotoMenu(QWidget *gotoMenu);

public slots:
	
	/**
	 * Goes to the frame with the number frameNumber on the framebar.
	 * @param frameNumber the number of the frame to go to.
	 */
	void gotoFrame(int frameNumber);
	
	/**
	 * Closes the goto menu and moves the focus to the mainwindowgui.
	 */
	void closeGotoMenu();
	
	/**
	 * Undoes the last undoable operation the user have done.
	 */
	void undo();
	
	/**
	 * Redoes the last undo the user have done.
	 */
	void redo();

	/**
	 * Copies and deletes the section.
	 */
	void cut();

	/**
	 * Copies a selection to the global clipboard.
	 */
	void copy();
	
	/**
	 * Pastes a selection from the global clipboard to the program.
	 */
	void paste();
	
private:
	QStatusBar *statusBar;
	FrameBar *frameBar;
	QWidget *gotoMenu;
	
signals:
	/**
	 * Adds the frames pointed to by a list.
	 * @param l the list containing pointers to the frames
	 */
	void addFrames(const QStringList &l);
	/**
	 * Removes the frames in the selection.
	 */
	void removeFrames();
	/**
	 * Undo or redo has happened
	 */
	void undoOrRedo();
};

#endif

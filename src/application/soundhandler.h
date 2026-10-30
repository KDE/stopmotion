/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SOUNDHANDLER_H
#define SOUNDHANDLER_H

#include "src/config.h"

#include <QObject>
#include <QStatusBar>
#include <QListWidget>
#include <QString>

class FrameBar;

/**
 * This class handles request related to sounds (adding, removing, etc).
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class SoundHandler : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates and initializes the Soundhandler.
	 * @param parent the parent of the soundhandler.
	 * @param sb a statusbar for displaying status information to the user.
	 * @param homeDir the home directory of the user.
	 * @param name the name of the soundhandler
	 */
	SoundHandler ( QObject *parent = nullptr, QStatusBar *sb = nullptr,
			const FrameBar* frameBar = nullptr, const QString& homeDir = QString(),
			const char *name = nullptr );
	
	/**
	 * Add the soundsList to the handler so that it can be cleared and filled as
	 * needed.
	 * @param soundsList the listbox to add to the handler.
	 */
	//void setSoundsList(Q3ListBox *soundsList);
	void setSoundsList(QListWidget *soundsList);
	
public slots:

	/**
	 * Attaches a sound to a frame in the animation.
	 */
	void addSound();
	
	/**
	 * Removes a sound from a frame in the animation.
	 */
	void removeSound();
	
	/**
	 * Sets the name of a sound in a frame in the animation.
	 */
	void setSoundName();

private:
	QStatusBar *statusBar;
	const FrameBar* frameBar;
	QString homeDir;
	//Q3ListBox *soundsList;
	QListWidget *soundsList;
};

#endif

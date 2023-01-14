/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
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
#ifndef MODELHANDLER_H
#define MODELHANDLER_H

#include <QObject>

class FrameBar;
class QStatusBar;
class QFileDialog;
class QPushButton;
class QString;

/**
 * This class handles request related to adding, removing and moving things
 * in the model.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ModelHandler : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates and initializes the ModelHander.
	 * @param parent the parent of the ModelHander.
	 * @param sb a statusbar for displaying status information to the user.
	 * @param frameBar the framebar containing the thumbnails
	 * @param lastVisitedDir the last visited directory
	 * @param name the name of the ModelHander
	 */
	ModelHandler( QObject *parent = 0, QStatusBar *sb = 0, FrameBar *frameBar = 0,
			QString *lastVisitedDir = 0, const char *name = 0 );

	/**
	 * Cleans up after the modelhandler.
	 */
	~ModelHandler();

	/**
	 * Function for setting the button used for removing frames. This button is
	 * needed for checking if it is enabled when attempting to remove frames with
	 * the delete key.
	 * @param removeFramesButton the button used for removing frames.
	 */
	void setRemoveFramesButton(QPushButton *removeFramesButton);


public slots:
	/**
	 * Brings up a dialog so that the user can choose a file to load.
	 */
	void chooseFrame();

	/**
	 *Adds the file fileName to the animation through the use of
	 *addFrames(QStringList).
	 *@param fileName the file to be added to the animation.
	 */
    void addFrame( const QString &fileName );

	/**
	 * Removes the currently selected frame from the animation.
	 */
	void removeFrames();

	/**
	 * Adds the files to the animation.
	 * @param fileNames the files to be added to the animation
	 */
	void addFrames(const QStringList & fileNames);


	/**
	 * Creates a new scene. This will be added after the selected frame.
	 */
	void newScene();

	/**
	 * Removes the active scene.
	 */
	void removeScene();

	 /**
	 * Launch the gimp to edit the current frame.
	 * @return 0 If successful, 1 on error.
	 */
	int editCurrentFrame();

private:
	FrameBar *frameBar;
	QStatusBar *statusBar;
	QPushButton *removeFramesButton;
	QFileDialog* fileDialog;
	QString *lastVisitedDir;

signals:
	/**
	 * Notified when the model is changed.
	 */
	void modelChanged();
};

#endif

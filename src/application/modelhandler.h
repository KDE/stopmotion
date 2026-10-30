/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
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
	ModelHandler( QObject *parent = nullptr, QStatusBar *sb = nullptr, FrameBar *frameBar = nullptr,
			QString *lastVisitedDir = nullptr, const char *name = nullptr );

	/**
	 * Cleans up after the modelhandler.
	 */
	~ModelHandler() override;

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

/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors;              *
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
#ifndef FRAMEBAR_H
#define FRAMEBAR_H

#include "src/presentation/observer.h"
#include "src/presentation/frontends/selection.h"

#include <QScrollArea>

#include <vector>

class Frame;
class ThumbView;
class FramePreferencesMenu;

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QResizeEvent;

/**
 * The framebar widget for viewing the animation model.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FrameBar : public QScrollArea, public Observer, public Selection {
	Q_OBJECT
public:
	/**
	 * Creates and sets up the framebar.
	 * @param parent the parent widget
	 */
	FrameBar(QWidget *parent = 0);

	/**
	 * Cleans up after the framebar.
	 */
	~FrameBar();

	/**
	 * Add an observer for the active scene changing. Any previously-set
	 * observer is unset.
	 * @param observer New observer to set. Ownership is not passed.
	 */
	void setObserver(ActiveFrameObserver* observer);

	/**
	 * Receives notification when a frame is added.
	 */
	void updateAdd(int scene, int index, int numFrames);

	/**
	 * Function to receive notification when one or more frames are deleted.
	 * @param fromFrame the first frame in the selection
	 * @param toFrame the last frame in the selection
	 */
	void updateRemove(int scene, int fromFrame, int toFrame);

	/**
	 *Function to receive notification when one or more frames are moved.
	 */
	void updateMove(int fromScene, int fromFrame, int count,
			int toScene, int toFrame);

	/**
	 *Function to receive notification when a new frame is selected.
	 */
	void updateNewActiveFrame(int scene, int frame);

	/**
	 * Function to receive notification when the model is erased.
	 */
	void updateClear();

	/**
	 * Receives notification when a frame is to be played.
	 * @param scene The scene containing the frame to be played.
	 * @param frame Index of the frame within the scene to be played.
	 */
	void updatePlayFrame(int scene, int frame);

	/**
	 * Function to recieve notification when a new scene is added to the
	 * model.
	 * @param index the index of the new scene.
	 */
	void updateNewScene(int index);

	/**
	 * Function to recieve notification when a scene is removed from
	 * the model.
	 * @param sceneNumber the scene which has been removed from the model.
	 */
	void updateRemoveScene(int sceneNumber);

	/**
	 * Function which receives notification when a scene in the animation
	 * has been moved and moves the icons in the framebar accordingly.
	 * @param sceneNumber the scene which have been moved.
	 * @param movePosition the position the scene has been moved to.
	 */
	void updateMoveScene(int sceneNumber, int movePosition);

	/**
	 * Function which receives notification when a scene is selected as the
	 * active scene in the animationmodel.
	 * @param sceneNumber the new scene to be set as the active scene in the
	 * framebar.
	 * @param framePaths paths to the pictures in the scene.
	 * @param frontend the frontend for getting a progressbar when adding
	 * opening the new active scene.
	 */
	void updateNewActiveScene(int sceneNumber);

	/**
	 * Updates the framebar when an external program has altered the disk files.
	 * @param sceneNumber The scene containing the changed frame.
	 * @param frameNumber the frame whose disk representation has been changed.
	 */
	void updateAnimationChanged(int sceneNumber, int frameNumber);

	/**
	 * Set whether the user is engaged selecting several frames or not (pressing
	 * shift).
	 * @param selecting true if the user is currently selecting multiple pictures
	 */
	void setSelecting(bool selecting);

	/**
	 * Returns true if the user is currently selecting several thumbviews.
	 * @return true if the user is currently selecting several thumbviews.
	 */
	bool isSelecting() const;

	/**
	 * Sets a selection of frames between (including) this frame and the activeFrame.
	 * @todo change name to setSelectionFrame
	 * @param selectionFrame the other border frame of the selection in addition to activeFrame
	 */
	void setSelection(int selectionFrame);

	/**
	 * Returns The frame number of the anchor of the selection.
	 * @return The anchor of the current selection, or returns the same value
	 * as {@ref getActiveFrame} if there is no selection.
	 */
	int getSelectionFrame() const;

	/**
	 * Returns the current active frame.
	 */
	int getActiveFrame() const;

	/**
	 * Returns the current active scene.
	 */
	int getActiveScene() const;

	/**
	 * Registers the frame preferences menu in the framebar.
	 * @param preferencesMenu the frame preferences menu.
	 */
	void setPreferencesMenu(FramePreferencesMenu *preferencesMenu);

	/**
	 * Displays the frame preferences for the active frame.
	 */
	void showPreferencesMenu();

	/**
	 * Retrieves the value of the movingScene property specifying which scene
	 * is currently being moved in the framebar.
	 * @return the value of the movingScene property.
	 */
	int getMovingScene() const;

	/**
	 * Sets the value of the movingScene property specifying which scene
	 * is currently being moved in the framebar.
	 * @param movingScene the new value of the movingScene property.
	 */
	void setMovingScene(int movingScene);

	/**
	 * Sets whether the scene is currently opening so that close requests
	 * can be ignored while it is processing.
	 *
	 * @param openingScene true if the scene is currently being opened. False
	 * if not.
	 */
	void setOpeningScene(bool openingScene);

	/**
	 * Returns true if a scene is currently being opened.
	 * @return true if a scene is currently being opened.
	 */
	bool isOpeningScene() const;

	int getFrameWidth() const;
	int getFrameHeight() const;
	int getSpace() const;

protected:
	/**
	 * Overloaded event function for when a drag enter occurs in the framebar
	 * @param event information about the dragEnterEvent
	 */
	void dragEnterEvent(QDragEnterEvent *event);

	/**
	 * Overloaded event function for when a drop event occurs in the framebar.
	 * @param event information about the dropEvent
	 */
	void dropEvent(QDropEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);

	void resizeEvent(QResizeEvent *event);

public slots:
	/**
	 * Receives notification when the sounds in a frame have been changed.
	 */
	void frameSoundsChanged();

signals:
	//The signals in the framebar is used for signaling small widget,
	//who are deemed to unsignificant to be observers, of changes in
	//the framebar/model. A bit hacky and should be in it's own widget.
	void newActiveFrame( const QString & );
	void newActiveFrame( int value );
	void modelSizeChanged( int modelSize );
	void newMaximumValue(int value);

private slots:
	void scroll();

private:
	static const int FRAME_HEIGHT = 88;
	static const int FRAME_WIDTH = 117;
	static const int SPACE = 2;

	/** Observer that wants to know when the active frame changes */
	ActiveFrameObserver* activeFrameObserver;

	/** Vector of thumbviews to keep track of the pictures in the framebar*/
	vector<ThumbView*>thumbViews;

	/** The active frame in the framebar*/
	int activeFrame;

	/** The active scene in the framebar */
	int activeScene;

	/** The scene which are being moved when draging a scene */
	int movingScene;

	/** The other border frame in a selection together with activeFrame.
	*   If only one is selected selectionFrame == activeFrame            */
	int selectionFrame;

	/** Direction to scroll when dragging.
	 * -1 = scroll negative, 0 = no scroll, 1 = scroll positive */
	int scrollDirection;

	int lowerScrollAreaX;
	int upperScrollAreaX;

	int lowerAccelScrollAreaX;
	int upperAccelScrollAreaX;

	int minScrollAreaX;
	int maxScrollAreaX;

	int minScrollAreaY;
	int maxScrollAreaY;

	/** True if the user is currently holding down shift to select multiple frames */
	bool selecting;

	bool openingScene;

	QTimer *scrollTimer;
	QScrollBar *scrollBar;

	QWidget *mainWidget;

	/** Pointer to the frame preferencesMenu */
	FramePreferencesMenu *preferencesMenu;

	/**
	 *Adds the picture to the frame to the framebar at position index.
	 *
	 *@param frame the path of the frame to be added to the framebar.
	 *@param index the place to add the frame.
	 */
	void addFrame(Frame* frame, unsigned int index);

	/**
	 * Adds the frames in the framesvector to the framebar.
	 * @param index The location to add the frames to.
	 * @param numFrames The number of frames to be added.
	 */
	void addFrames(int index, int numFrames);

	/**
	 * Removes a selection of thumbviews from the framebar.
	 * @param fromFrame the first thumbview to remove.
	 * @param toFrame the last thumbview to remove.
	 */
	void removeFrames(int fromFrame, int toFrame);

	/**
	 * Move the frames from fromFrame (inclusive) to toFrame to movePosition.
	 * @param fromFrame the first frame to move.
	 * @param toFrame the last frame to move.
	 * @param movePosition the position to move the frames to.
	 */
	void moveFrames(int fromFrame, int toFrame, int movePosition);

	/**
	 *Sets the thumbview frameNumber in the framebar to be the active frame.
	 *@param frameNumber the number of the new active frame
	 */
	void setActiveFrame(int frameNumber);

	/**
	 * Creates a new scene and adds it to the framebar.
	 * @param index the location the new scene should be added at.
	 */
	void newScene(int index);

	/**
	 * Removes the scene with index sceneNumber from the framebar.
	 * @param sceneNumber the scene to be removed from the framebar.
	 */
	void removeScene(int sceneNumber);

	/**
	 * Moves a scene in the framebar.
	 * @param sceneNumber the scene that are moved.
	 * @param movePosition the position the scene are moved to.
	 */
	void moveScene(int sceneNumber, int movePosition);

	/**
	 * Sets a new scene as the active scene in the framebar and opens it.
	 * @param sceneNumber the scene to set as the active scene.
	 * @param framePaths the paths to the frames in the scene.
	 * @param frontend the frontend for getting a progressbar when adding the
	 * frames to the scene.
	 */
	void setActiveScene(int sceneNumber);

	/**
	 *Moves a thumbview in the thumbviews-vector from fromPosition to toPosition.
	 *@param fromPosition the place in the vector the thumbview is moved from.
	 *@param toPosition the place in the vector the thumbview is moved to.
	 */
	void moveThumbView(int fromPosition, int toPosition);

	/**
	 * Sends an {@c updateNewActiveFrame} to the observer, if appropriate.
	 */
	void updateObserver();
};

#endif

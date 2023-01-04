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
#ifndef THUMBVIEW_H
#define THUMBVIEW_H

#include <QLabel>

class FrameBar;

class QDropEvent;
class QWidget;

/**
 * This abstract class represents the thumbviews in the framebar. It has
 * two subclasses: the FrameThumbView and the SceneThumbView.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ThumbView : public QLabel {
	Q_OBJECT
public:

	/**
	 * Creates and sets up the thumbview class.
	 * @param frameBar the framebar for communicating.
	 * @param parent the parent widget.
	 * @param number the number of this thumbview in the framebar.
	 */
	ThumbView(FrameBar *frameBar, QWidget *parent, int number, const char *name = 0);

	/**
	 * Cleans up after the widget.
	 */
	virtual ~ThumbView();

	/**
	 * Adds another reference. The reference count starts at 1 on construction.
	 */
	void addRef();

	/**
	 * Removes a reference. If the reference count falls to zero this object is
	 * deleted.
	 */
	void delRef();

	/**
	 * Function to set the number of this ThumbView in the framebar
	 * when the ThumbView change position.
	 * @param number the new number for the ThumbView in the framebar.
	 */
	virtual void setNumber(int number);

	/**
	 *Retrieves the number of this ThumbView in the framebar
	 *@return the number for this ThumbView in the framebar
	 */
	int getNumber() const;

	/**
	 * Abstract function for telling a framethumbview that it has one or more sounds
	 * attached to it.
	 * @param hasSounds true if the frame has one or more sounds attached to it.
	 */
	virtual void setHasSounds(bool hasSounds);

	/**
	 * Sets whether a framethumbview should be marked as selected.
	 * @param selected whether this framethumbview should be showed as selected.
	 */
	virtual void setSelected(bool selected);

	/**
	 * Abstract function for telling a scenethumbview whether the scene is opened or
	 * closed
	 * @param isOpened true if the scene is opened.
	 */
	virtual void setOpened(bool isOpened);

	/**
	 * Notifies the thumbview that a drop have happened inside its borders.
	 *
	 * The reason it doesn't accept it itself is that the framebar need the information
	 * so that it can autoscroll.
	 * @param event information about the event.
	 */
	virtual void contentsDropped(QDropEvent * event);

	FrameBar* getFrameBar() const {
		return frameBar;
	}

private:
	/**
	 * Reference count. As a QDrag object might be referring to this object
	 * after it has been deleted from the frame bar, we need to delay deletion
	 * until a drag starting here has ended.
	 */
	int refCount;

	/** The framebar for communicating with it */
	FrameBar *frameBar;

	/** For a scene, the scene number within the animation. For a frame, the
	 * frame number within the scene. */
	int number;
};

#endif

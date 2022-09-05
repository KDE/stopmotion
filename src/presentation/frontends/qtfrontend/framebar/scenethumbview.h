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
#ifndef SCENETHUMBVIEW_H
#define SCENETHUMBVIEW_H

#include "src/presentation/frontends/qtfrontend/framebar/thumbview.h"

class SceneArrowButton;
class FrameBar;

class QObject;
class QWidget;
class QMouseEvent;
class QPaintEvent;
class QDropEvent;

/**
 * Custom widget representing a scene in the framebar.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class SceneThumbView : public ThumbView {
	Q_OBJECT
public:
	/**
	 * Initializes the SceneThumbView widgets attributes.
	 * @param frameBar the frameBar for communication.
	 * @param parent the parent of this widget.
	 * @param number the number of the scene this widget represents.
	 */
	SceneThumbView(FrameBar *frameBar, QWidget *parent = 0, int number = 0, const char * name = 0);

	/**
	 * Clean up after the SceneThumbView.
	 */
	~SceneThumbView();

	/**
	 * Sets if the scene is opened or closed.
	 * @param isOpened true if the scene is opened.
	 */
	void setOpened(bool isOpened);

	/**
	 * Returns whether the scene is opened.
	 * @return true if the scene is currently opened.
	 */
	bool getIsOpened() const;

public slots:
	/**
	 * Closes the scene.
	 */
	void closeScene();

protected:
	/**
	 * Overloaded function to paint the widget.
	 * @param p information about the paintEvent.
	 */
	virtual void paintEvent ( QPaintEvent *p );

	/**
	 * Overloaded event function to receive mousepress-events.
	 * @param e information about the mousepress-event.
	 */
	void mousePressEvent( QMouseEvent * e );

	/**
	 * Overloaded event function to receive mouserelease events.
	 * @param e information about the event.
	 */
	void mouseReleaseEvent( QMouseEvent * e );

	/**
	 * Overloaded event function to receive mouseMoveEvents in the scenethumbview.
	 * Used for moving scenes.
	 * @param me information about the mouseMoveEvent.
	 */
	void mouseMoveEvent(QMouseEvent *me);

	/**
	 * Notifies the scenethumbview that a drop have happened inside its borders.
	 * @param event information about the event.
	 */
	virtual void contentsDropped(QDropEvent * event);

private:
	/** Coordinate for calculating when a drag should start */
	QPoint dragPos;

	/** Whether the scene is opened or not. */
	bool isOpened;

	/** The scene arrow button for closing the scene. */
	SceneArrowButton *arrowButton;

	/** The image displayed on the widget */
	QPixmap centerIcon;

	/** The font of the text on the widget */
	QFont f;

	/**
	 * Starts a drag event with this scene as the drag object.
	 */
	void startDrag();
};

#endif

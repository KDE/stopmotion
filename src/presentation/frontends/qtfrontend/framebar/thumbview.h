/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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

#include "framebar.h"

#include <qlabel.h>

class FrameBar;

/**
 * This abstract class represents the thumbviews in the framebar. It has 
 * two subclasses: the FrameThumbView and the SceneThumbView.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ThumbView : public QLabel 
{
public:

	/**
	 * Creates and sets up the thumbview class.
	 * @param frameBar the framebar for communicating.
	 * @param parent the parent widget.
	 * @param number the number of this thumbview in the framebar.
	 */
	ThumbView(FrameBar *frameBar, QWidget *parent, int number, const char * name = 0);
	
	/**
	 * Cleans up after the widget.
	 */
	~ThumbView();
	
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
	int getNumber();
	
	/**
	 * Abstract function for telling a framethumbview that it has one or more sounds
	 * attatched to it.
	 * @param hasSounds true if the frame has one or more sounds attatched to it.
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
	
	/**
	 * Resizes the thumbview.
	 * @param height the new height of the widget.
	 */
	virtual void resizeThumb(int height);
	
protected:
	/** The framebar for communicating with it */
	FrameBar *frameBar;
	
	/**The number this frame has in the framebar*/
	int number;
	
	/**Coordinate for calculating when a drag should start to give some slack*/
	QPoint dragPos;
};

#endif

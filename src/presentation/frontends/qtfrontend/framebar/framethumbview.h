/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FRAMETHUMBVIEW_H
#define FRAMETHUMBVIEW_H

#include <QPoint>
#include <QString>

#include "thumbview.h"

class FrameBar;
class QMouseEvent;
class QDropEvent;
class QPaintEvent;
class QWidget;

/**
 * Widget representing a frame in the animation.
 * 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FrameThumbView : public ThumbView {
	Q_OBJECT
public:
	/**
	 * Creates and sets up the framethumbview.
	 * @param frameBar the framebar for communicating with it.
	 * @param parent the parent widget.
	 * @param number the number of this widget in the framebar.
	 */
	FrameThumbView(FrameBar *frameBar, QWidget *parent = nullptr, int number = 0, const char *name = nullptr);
	
	/**
	 * Cleans up after the widget.
	 */
	~FrameThumbView() override;
	
	/**
	 * Overloaded event function to receive mousepress-events.
	 * @param e information about the mousepress-event.
	 */
	void mousePressEvent( QMouseEvent * e ) override;
	
	/**
	 * Overloaded event function to receive mouserelease-events.
	 * @param e information about the mouserelease-event.
	 */
	void mouseReleaseEvent( QMouseEvent * e ) override;
	
	/**
	 * Overloaded event function to receive mouseMoveEvents.
	 * @param me information about the mouseMoveEvent. 
	 */
	void mouseMoveEvent(QMouseEvent *me) override;
	
	/**
	 * Overloaded event function to receive events when the user double clicks
	 * in the thumbview. Double clicks here cause the preferences menu for this
	 * frame to be shown.
	 * @param e information about the mouseDoubleClickEvent.
	 */
	void mouseDoubleClickEvent ( QMouseEvent * e ) override;
	
	/**
	 * Sets/changes the number of this widget.
	 * @param number the new number of this widget.
	 */
	void setNumber(int number) override;
	
	/**
	 * Sets whether this widget has sounds.
	 * @param hasSounds 
	 */
	void setHasSounds( bool hasSounds ) override;
	
	/**
	 * Sets whether this thumbview should be selected.
	 * @param selected whether this frame is selected.
	 */
	void setSelected(bool selected) override;
	
	/**
	 * Notifies the framethumbview that a drop have happened inside its borders. 
	 * @param event information about the event.
	 */
	void contentsDropped(QDropEvent * event) override;
	
protected:
	/**
	 * Event function which paints the widget.
	 */
	void paintEvent ( QPaintEvent * ) override;
	
private:	
	/** Coordinate for calculating when a drag should start */
	QPoint dragPos;

	QString stringNumber;
	
	/** The width the text should have. (Cached for efficiency reasons)*/
	int textWidth;
	
	/** Whether this frame has sounds attached to it */
	bool hasSounds;
	
	/** Specifies whether this frame is selected. */
	bool selected;
	
	/** Starts an uri drag of the picture in this label. */
	void startDrag();
};

#endif

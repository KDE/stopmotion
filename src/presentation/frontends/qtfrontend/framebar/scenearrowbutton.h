/*
 * SPDX-FileCopyrightText: 2005-2008 fredrik <fredrik@Elanor>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SCENEARROWBUTTON_H
#define SCENEARROWBUTTON_H

#include <QWidget>
#include <QTimer>
//Added by qt3to4:
#include <QMouseEvent>
#include <QEnterEvent>
#include <QEvent>
#include <QPaintEvent>


/**
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class SceneArrowButton : public QWidget
{
	Q_OBJECT
public:
	/**
	 * Sets up the scenearrowbutton
	 * @param parent the parent of the widget.
	 */
	SceneArrowButton( QWidget *parent=nullptr );
	
	/**
	 * Sets if the scene is opened or closed.
	 * @param isOpened true if the scene is opened.
	 */
	void setOpened(bool isOpened);
	
public slots:
	/**
	 * Slot for moving the icon to create an animation-like effect.
	 */
	void moveIcon();
	
protected:
	/** Overloaded function to paint the widget.
	 * @param p information about the paintEvent.
	 */
	void paintEvent ( QPaintEvent *p) override;
	
	/**
	 * Overloaded event function to receive mouserelease-events.
	 * @param e information about the mouserelease-event.
	 */
	void mouseReleaseEvent( QMouseEvent * e ) override;
	
	/**
	 * Overloaded event function for receiving information when the mouse
	 * enter the widget
	 * @param e information about the event
	 */
	void enterEvent ( QEnterEvent * e) override;
	
	/**
	 * Overloaded event function for receiving information when the mouse
	 * leaves the widget
	 * @param e information about the event
	 */
	void leaveEvent ( QEvent * e) override;
	
private:
	bool isOpened;
	int iconX;
	
	QTimer *moveTimer;
	
signals:
	void clicked();
};

#endif

/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MENUFRAME_H
#define MENUFRAME_H

#include <QFrame>
#include <QSpinBox>
#include <QLineEdit>
#include <QMouseEvent>

/**
 * A customized GUI menu class for embedded menues which shouldn't be visible 
 * at all times. The menues are hidden and pops up at various places in the
 * gui interface. 
 *
 * This class allow you to create preferences/etc. menues without resorting to
 * the modal cover-the-gui menues.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class MenuFrame : public QFrame
{
	Q_OBJECT
public:
	/**
	 * Creates and sets up the menu frame. 
	 *
	 * @param parent the parent widget.
	 * @param name the name of the menu.
	 */
	MenuFrame(QWidget * parent = nullptr, const char * name = nullptr);
			
	/**
	 * Sets the focus widget to the QSpinBox: focusSpinBox.
	 *
	 * NB: One should only use use one of the two setFocusWidget functions. If both
	 *     are used the spinbox will be used. (A bit hacky but saves me from creating
	 *     a custom(/messy!) class hierarchy and gives me time to go out and have  
	 *     a few beers tonight).
	 *
	 * @param focusSpinBox the focuswidget of this menu.
	 */
	void setFocusWidget(QSpinBox * focusSpinBox = nullptr);
	
	
	/**
	 * Sets the focus widget to the QLineEdit: focusSpinBox.
	 *
	 * NB: One should only use use one of the two setFocusWidget functions. If both
	 *     are used the spinbox will be used. (A bit hacky but saves me from creating
	 *     a custom(/messy!) class hierarchy and gives me time to go out and have  
	 *     a few beers tonight).
	 *
	 * @param focusLineEdit the focuswidget of this menu.
	 */
	void setFocusWidget(QLineEdit * focusLineEdit = nullptr);
	
protected:
	/**
	 * Overloaded function to intercept when the user press the mouse inside 
	 * the menu. (So that the mainwindow doesn't get the event).
	 * @param e information about the event.
	 */
	void mousePressEvent( QMouseEvent * e) override;
	
private:
	QSpinBox *focusSpinBox;
	QLineEdit *focusLineEdit;
	
public slots:
	/**
	 * Opens the menu.
	 */
	virtual void open();
	
	/**
	 * Closes the menu and turns the focus to the newFocusWidget.
	 * @param newFocusWidget the widget who will receive the focus after this menu
	 * closes.
	 */
	void close(QWidget * newFocusWidget);
};

#endif

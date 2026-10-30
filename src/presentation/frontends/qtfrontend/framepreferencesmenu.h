/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FRAMEPREFERENCESMENU_H
#define FRAMEPREFERENCESMENU_H

#include "menuframe.h"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

class SoundHandler;
class FrameBar;

/**
 * A customized GUI menu class for the frame preferences menu.
 *
 * Inherits from MenuFrame but adds some functionality around loading the
 * present preferences when opened.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FramePreferencesMenu : public MenuFrame {
	Q_OBJECT
public:
	
	/**
	 * Creates and sets up the preferences menu. 
	 * @param parent the parent widget
	 * @param soundHandler the sound handler
	 * @param frameBar The frame bar.
	 * @param name the name of the menu
	 */
	FramePreferencesMenu( QWidget * parent = nullptr, SoundHandler *soundHandler = nullptr,
			const FrameBar *frameBar = nullptr, const char * name = nullptr);

	~FramePreferencesMenu() override;

	/**
	 * Retranslates the strings in the frame preferences menu.
	 */
	void retranslateStrings();
	
private:
	SoundHandler *soundHandler;
	const FrameBar *frameBar;
	QListWidget *soundsList;
	QLabel *soundsLabel;
	QPushButton *closeButton;
	QPushButton *addSoundButton;
	QPushButton *removeSoundsButton;
	QPushButton *changeNameButton;
	QGridLayout *grid;
	
public slots:
	/**
	 * Opens the menu.
	 */
	void open() override;
	
	/**
	 * Closes the menu and turns the focus to the newFocusWidget.
	 */
	void close();
};

#endif

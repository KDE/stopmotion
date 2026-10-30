/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FLEXIBLESPINBOX_H
#define FLEXIBLESPINBOX_H

#include <QSpinBox>

class QKeyEvent;

/**
 * An extended spin box class.
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class FlexibleSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	
	/**
	 * Constructs and initializes the object.
	 * @param parent the parent widget
	 */
	FlexibleSpinBox(QWidget *parent = nullptr);
	
	/**
	 * Checks what kind of key event the user has requested and 
	 * does actions based on this. 
	 * @param k the key event
	 */
	void keyPressEvent( QKeyEvent * k ) override;
	
public slots:
	/**
	 * Sets a limit for the maximum value. This is based on the
	 * the thumbnails displayed in the framebar. 
	 * @param maxValue maximum value (index of the last frame in framebar)
	 */
	void setMaximumValue(int maxValue);

	/**
	 * Sets the value to one more than @a index.
	 */
	void setIndexSecond(int, int index);

signals:
	/**
	 * Emits this signal when pressing enter.
	 * @param frameNumber the chosen frame number in this spinbox
	 */
	void spinBoxTriggered(int frameNumber);
	
	/**
	 * Emits this signal when getting an escape event (Key_Escape).
	 */
	void spinBoxCanceled();

protected:
	void showEvent(QShowEvent * event) override;
};

#endif

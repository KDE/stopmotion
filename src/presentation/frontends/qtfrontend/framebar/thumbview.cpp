/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "thumbview.h"

class QWidget;

ThumbView::ThumbView(FrameBar *frameBar, QWidget *parent, int number,
		const char *name)
	: QLabel(parent), refCount(1), frameBar(frameBar), number(number) {
	setObjectName(name);
}


ThumbView::~ThumbView()
{
}


void ThumbView::setNumber(int number)
{
	this->number = number;
}


int ThumbView::getNumber() const
{
	return number;
}


void ThumbView::setHasSounds(bool) {}
void ThumbView::setOpened(bool) {}
void ThumbView::setSelected(bool) {}
void ThumbView::contentsDropped(QDropEvent *) {
}

void ThumbView::addRef() {
	++refCount;
}

void ThumbView::delRef() {
	--refCount;
	if (refCount == 0)
		deleteLater();
}

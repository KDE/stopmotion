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

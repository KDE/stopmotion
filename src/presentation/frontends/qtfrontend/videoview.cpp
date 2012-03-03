/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com     & fredrikbk@hotmail.com                *
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

#include "videoview.h"



VideoView::VideoView( QWidget *parent, const char *name, WFlags f)
		: QWidget(parent, name, f)
{
	this->mixCount = 2;
}


VideoView::~VideoView()
{
}


void VideoView::initCamera()
{
}


bool VideoView::on()
{
	return false;
}


void VideoView::off()
{
}


void VideoView::capture()
{
}


void VideoView::setPlaybackSpeed(int)
{
}

void VideoView::initCompleted()
{
	emit cameraReady();
}


void VideoView::updateAdd(const vector<char*>&, unsigned int, Frontend*)
{
}


void VideoView::updateRemove(unsigned int, unsigned int)
{
}


void VideoView::updateMove(unsigned int, unsigned int, unsigned int)
{
}


void VideoView::updateNewActiveFrame(int)
{
}


void VideoView::updateClear()
{
}


void VideoView::updatePlayFrame(int)
{
}


void VideoView::updateNewScene(int)
{
}


void VideoView::updateRemoveScene(int)
{
}


void VideoView::updateMoveScene(int, int)
{
}


void VideoView::updateNewActiveScene(int, vector<char*>,
	Frontend *)
{
}


void VideoView::updateAnimationChanged(vector<Frame*>&)
{
}


void VideoView::setMixCount(int mixCount)
{
	this->mixCount = mixCount;
}


bool VideoView::setViewMode(int)
{
	return false;
}


int VideoView::getViewMode()
{
	return -1;
}

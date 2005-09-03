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
#ifndef VIDEOFACTORY_H
#define VIDEOFACTORY_H

#include "videoencoder.h"
#include "src/technical/projectserializer.h"
#include "src/presentation/frontends/frontend.h"


/**
 * Class for exporting a stopmotion project ccntaining images and sounds to
 * to a video file. 
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class VideoFactory
{
public:
	/**
	 * Registers a serializer that will be used to gather information about
	 * image paths etc.
	 * @param serializer the serializer used by stopmotion
	 * @param frontend the frontend to display information to
	 */
	VideoFactory(ProjectSerializer *serializer, Frontend *frontend);
	~VideoFactory();
	
	/**
	 * Creates the video file based on the passed encoder. The encoder holds
	 * information needed to complete the export.
	 * @param encoder the encoder to be used for video export
	 * @return path to the created video file
	 */
	const char* createVideoFile(VideoEncoder *encoder);

private:
	ProjectSerializer *serializer;
	Frontend *frontend;
	
	int startEncoder(const char* command);
};

#endif

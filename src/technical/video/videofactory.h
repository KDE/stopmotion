/***************************************************************************
 *   Copyright (C) 2005-2014 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

class VideoEncoder;
class AnimationImpl;

/**
 * Class for exporting a stopmotion project ccntaining images and sounds to
 * to a video file.
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class VideoFactory {
public:
	/**
	 * Objects of this class make a video file.
	 * @param animation The animation to be turned into a video
	 */
	VideoFactory(const AnimationImpl *animation);
	~VideoFactory();

	/**
	 * Creates the video file based on the passed encoder. The encoder holds
	 * information needed to complete the export.
	 * @param encoder The encoder to be used for video export
	 * @param playbackSpeed The frame rate (in frames per second) at which
	 * the video should be encoded, assuming the video encoder supports
	 * this adjustment.
	 * @return path to the created video file
	 */
	const char* createVideoFile(VideoEncoder *encoder, int playbackSpeed);

private:
	const AnimationImpl *anim;
};

#endif

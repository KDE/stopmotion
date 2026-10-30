/*
 * SPDX-FileCopyrightText: 2005-2014 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

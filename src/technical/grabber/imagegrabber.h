/*
 * SPDX-FileCopyrightText: 2005-2008 Bjoern Erik Nilsen <bjoern.nilsen@bjoernen.com>
 * SPDX-FileCopyrightText: 2005-2008 Fredrik Berg Kjoelstad <fredrikbk@hotmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef IMAGEGRABBER_H
#define IMAGEGRABBER_H

/**
 * Abstract class for the different video grabbers.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImageGrabber {
	char* path;
	ImageGrabber(const ImageGrabber&) = delete;
	ImageGrabber& operator=(const ImageGrabber&) = delete;
public:
	/**
	 * Constructs and initializes the object.
	 * @param filePath path to the output file grabbed from a device.
	 * Ownership is not passed.
	 */
	ImageGrabber(const char* filePath);
	virtual ~ImageGrabber() = 0;

	/**
	 * Checks if the process is running in daemon mode.
	 * @return true if it runs in daemon mode, false otherwise
	 */
	virtual bool isGrabberProcess() = 0;

	virtual bool setPrePollCommand(const char *command) = 0;
	virtual bool setStartCommand(const char *command) = 0;
	virtual bool setStopCommand(const char *command) = 0;

	/**
	 * Abstract function for initializing the grabber.
	 * @return true on success, false otherwise
	 */
	virtual bool init() = 0;

	/**
	 * Abstract function for shutting down the grabber.
	 * @return true on success, false otherwise
	 */
	virtual bool tearDown() = 0;

	/**
	 * Abstract function for grabbing an image.
	 * @return true on success, false otherwise
	 */
	virtual bool grab() = 0;

	/**
	 * Returns the path used to construct this object.
	 */
	const char* filePath() const;
};

#endif

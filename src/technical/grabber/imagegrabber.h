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
#ifndef IMAGEGRABBER_H
#define IMAGEGRABBER_H


/**
 * Abstract class for the different video grabbers used by the SDLVideoView
 * widget.
 *
 * @author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class ImageGrabber 
{
public:
	/**
	 * Constructs and initializes the object.
	 * @param filePath path to the output file grabbed from a device
	 * @param isProcess true if the process is running in deamon mode, false otherwise
	 */
	ImageGrabber(char* filePath, bool isProcess = false);
	virtual ~ImageGrabber() {};
	
	/**
	 * Checks if the process is running in deamon mode.
	 * @return true if it runs in deamon mode, false otherwise
	 */
	bool isGrabberProcess();
	
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
	
protected:
	char* filePath;
	bool isProcess;
};

#endif

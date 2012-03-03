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
#ifndef PICTUREPREVIEW_H
#define PICTUREPREVIEW_H

#include <qlabel.h>
#include <qfiledialog.h> 


/**
 *@author Bjoern Erik Nilsen & Fredrik Berg Kjoelstad
 */
class PicturePreview : public QLabel, public QFilePreview {
public:
	/**
	 * Sets up the filepreview-label.
	 * @param parent the parent of the widget.
	 */
	PicturePreview( QWidget *parent=0 );

	/**
	 * Displays the preview picture
	 * @param u the file path to the selected file in the filechooser.
	 */
	void previewUrl( const QUrl &u );

private:
	static const int PREVIEW_SIZE = 100;
};

#endif

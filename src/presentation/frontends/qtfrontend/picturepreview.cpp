/***************************************************************************
 *   Copyright (C) 2005 by fredrik                                         *
 *   fredrik@Elanor                                                        *
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
#include "picturepreview.h"

#include <qpixmap.h> 
#include <qimage.h> 

PicturePreview::PicturePreview( QWidget *parent ) : QLabel(parent)
{
	this->setMinimumWidth(60);
	this->setMaximumSize(PREVIEW_SIZE, PREVIEW_SIZE);
}


void PicturePreview::previewUrl( const QUrl &u )
{
	QString path = u.path();

	QImage img( path );
	
	if( !img.isNull() ) {
		img = img.smoothScale(PREVIEW_SIZE, PREVIEW_SIZE);
		QPixmap pix;
		pix.convertFromImage(img);
		setPixmap( pix );
	}
}

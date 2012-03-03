/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern_erik_nilsen@hotmail.com & fredrikbk@hotmail.com                *
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
#include "videofactory.h"


VideoFactory::VideoFactory(ProjectSerializer *serializer, Frontend *frontend)
		: serializer(serializer), frontend(frontend)
{

}


const char* VideoFactory::createVideoFile(VideoEncoder *encoder)
{
	string startCommand = encoder->getStartCommand();
	if ( startCommand.empty() == false ) {
		int index = startCommand.find("$IMAGEPATH");
		if (index != -1) {
			if ( serializer->getImagePath() ) {
				string s = serializer->getImagePath();
				string imagePath = s.substr(0, s.length() - 1);
				startCommand.replace(index, strlen("$IMAGEPATH"), imagePath);
			}
			else {
				return NULL;
			}
		}
		index = startCommand.find("$VIDEOFILE");
		if (index != -1) {
			if ( encoder->getOutputFile() ) {
				string outputFile = encoder->getOutputFile();
				startCommand.replace(index, strlen("$VIDEOFILE"), outputFile);
			}
			else {
				return NULL;
			}
		}
		if ( startEncoder(startCommand.c_str()) == 0 ) {
			return encoder->getOutputFile();
		}
	}
	return NULL;
}


int VideoFactory::startEncoder( const char * command )
{
	// run this in a separate process or thread?
	return system(command);
}

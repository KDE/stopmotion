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
#include "src/technical/util.h"

#include <ext/stdio_filebuf.h>
#include <istream>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>

using namespace std;


const char* Util::checkCommand(const char *command)
{
	assert(command != 0);
	
	int len = 7 + strlen(command);
	char tmp[len];
	snprintf(tmp, len, "which %s", command);
	
	FILE *fp = popen(tmp, "r");
	__gnu_cxx::stdio_filebuf<char> buf(fp, ios::in);
	istream bufStream(&buf);
	
	string line = "";
	getline(bufStream, line);
	if (line != "") {
		char *path = new char[line.length() + 1];
		strcpy(path, line.c_str());
		return path;
	}
	
	return NULL;
}


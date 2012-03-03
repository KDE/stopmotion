/***************************************************************************
 *   Copyright (C) 2005 by Bjoern Erik Nilsen & Fredrik Berg Kjoelstad     *
 *   bjoern.nilsen@bjoernen.com & fredrikbk@hotmail.com                    *
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
#include "packer.h"

#include <libtar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>


int create(char *tarFile, char *rootDir, libtar_list_t *l)
{
	TAR *t;
	
	if (tar_open(&t, tarFile, NULL, O_WRONLY | O_CREAT, 0644, 0 | 0) == -1 ) {
		fprintf(stderr, "tar_open(): %s\n", strerror(errno));
		return 1;
	}
	
	char *pathname = NULL;
	char buf[256] = {0};
	libtar_listptr_t lp;
	libtar_listptr_reset(&lp);
	
	while ( libtar_list_next(l, &lp) != 0 ) {
		pathname = (char *)libtar_listptr_data(&lp);
		
		if (pathname[0] != '/' && rootDir != NULL) {
			snprintf(buf, sizeof(buf), "%s/%s", rootDir, pathname);
		}
		else {
			snprintf(buf, sizeof(buf), pathname);
		}
		
		if ( tar_append_tree(t, buf, pathname) != 0 ) {
#ifndef NO_DEBUG
			fprintf(stderr,"tar_append_tree(\"%s\", \"%s\"): %s\n", 
					buf,pathname, strerror(errno));
#endif
			tar_close(t);
			return 1;
		}
	}
	
	if (tar_append_eof(t) != 0) {
#ifndef NO_DEBUG
		fprintf(stderr, "tar_append_eof(): %s\n", strerror(errno));
#endif
		tar_close(t);
		return 1;
	}

	if (tar_close(t) != 0) {
#ifndef NO_DEBUG
		fprintf(stderr, "tar_close(): %s\n", strerror(errno));
#endif
		return 1;
	}

	return 0;
}


int extract(char *tarFile, char *rootDir)
{
	TAR *t;

	if ( tar_open(&t, tarFile, NULL, O_RDONLY, 0, 0 | 0) == -1 ) {
#ifndef NO_DEBUG		
		fprintf(stderr, "tar_open(): %s\n", strerror(errno));
#endif
		return 1;
	}

	if (tar_extract_all(t, rootDir) != 0)
	{
#ifndef NO_DEBUG
		fprintf(stderr, "tar_extract_all(): %s\n", strerror(errno));
#endif
		return 1;
	}

	if (tar_close(t) != 0)
	{
#ifndef NO_DEBUG
		fprintf(stderr, "tar_close(): %s\n", strerror(errno));
#endif
		return 1;
	}
	
	return 0;
}


int pack(char *directory, char *toProjectFile)
{
	assert(directory != NULL);
	assert(toProjectFile != NULL);

	char tmp[256] = {0};
	
	int len = strlen(directory);
	if (directory[len - 1] == '/') {
		strncpy(tmp, directory, len - 1);
		strcat(tmp, "\0");
	}
	else {
		strcpy(tmp, directory);
	}
	
	// Need to do this because both dirname and basename may modify
	// the contents of tmp.
	char *tmpCopy1 = strdup(tmp);
	char *tmpCopy2 = strdup(tmp);
	
	char *rootDir = dirname(tmpCopy1);
	char *dir = basename(tmpCopy2);

	libtar_list_t *files = libtar_list_new(LIST_QUEUE, NULL);
	libtar_list_add(files, (void*)dir);
	int ret = create(toProjectFile, rootDir, files);
	
	free(tmpCopy1);
	free(tmpCopy2);
	
	return ret;
}


char* unpack(char *projectFile, char *rootDir)
{
	int ret = extract(projectFile, rootDir);
	if (ret == 1) {
		return NULL;
	}
	
	// The routine for detecting the path of the unpacked data
	// is not general. It is only guaranted to work with stopmotion.
	
	char tmp[256] = {0};
	DIR *dp = opendir(rootDir);
	struct dirent *ep;
	struct stat st;
	
	while ( (ep = readdir(dp)) ) {
		if ( strncmp(ep->d_name, ".", 1) != 0 ) {
			sprintf(tmp, "%s%s", rootDir, ep->d_name);
			stat(tmp, &st);
			// is a directory
			if ( S_ISDIR(st.st_mode) != 0) {
				char *dir = (char*)malloc( strlen(tmp) + 2 );
				strcpy(dir, tmp);
				strcat(dir, "/");
				return dir;
			}
		}
	}
	
	return NULL;
}

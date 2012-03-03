#include "config.h"
#include "src/domain/domainfacade.h"

#ifdef QTGUI
#include "presentation/frontends/qtfrontend/qtfrontend.h"
#endif

#include "presentation/frontends/nonguifrontend/nonguifrontend.h"
#include "presentation/frontends/frontend.h"
#include "src/foundation/preferencestool.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <stdio.h>
#include <libgen.h>


void init(Frontend *frontend, DomainFacade *facadePtr);
void cleanup();
bool isRecoveryMode();
void createDirectories();
void recover(DomainFacade *facadePtr);


int main(int argc, char **argv) 
{
	// returns a pointer that is allocated with new
	DomainFacade *facadePtr = DomainFacade::getFacade();
	Frontend *frontend;
	
	// if program is started with --nonGUI
	if ( argc > 1 && strcmp(argv[1], "--nonGUI") == 0 ) {
		frontend = new NonGUIFrontend(facadePtr); 
	}
	else {
#ifdef QTGUI
		frontend = new QtFrontend(argc, argv);
#endif
	}
	facadePtr->registerFrontend(frontend);
	
	init(frontend, facadePtr);
	atexit(cleanup);
	
	int ret = frontend->run(argc, argv);
	
	delete facadePtr;
	facadePtr = NULL;
	delete frontend;
	frontend = NULL;
	
	return ret;
}


// creates directories if they already not exist.
void init(Frontend *frontend, DomainFacade *facadePtr)
{
	if ( isRecoveryMode() ) {
		int ret = frontend->askQuestion(
				"Something caused Stopmotion to exit abnormally\n"
				"last time it was runned. Do you want to recover?");
		// The user wants to recover
		if (ret == 0) {
			recover(facadePtr);
		}
		else {
			cleanup();
			createDirectories();
		}
	}
	else {
		cleanup();
		createDirectories();
	}
}


// cleanup the directories
void cleanup()
{
	char command[256] = {0};
	sprintf( command, "rm -rf %s/.stopmotion/tmp/", getenv("HOME") );
	system(command);
	sprintf( command, "rm -rf %s/.stopmotion/trash/", getenv("HOME") );
	system(command);
	sprintf( command, "rm -rf %s/.stopmotion/packer/", getenv("HOME") );
	system(command);
}


bool isRecoveryMode()
{
	char tmp[256] = {0};
	sprintf( tmp, "%s/.stopmotion/tmp/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	sprintf( tmp, "%s/.stopmotion/trash/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	sprintf( tmp, "%s/.stopmotion/packer/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	
	// Everything is intact and we have to run in recovery mode
	return true;
}


void createDirectories()
{
	char tmp[256] = {0};
	sprintf( tmp, "%s/.stopmotion/", getenv("HOME") );
	
	// is ~/.stopmotion doesn't exist
	if ( access(tmp, F_OK) == -1 ) {
		mkdir(tmp, 0755);
	}
	
	sprintf( tmp, "%s/.stopmotion/tmp/", getenv("HOME") );
	mkdir(tmp, 0755);
	sprintf( tmp, "%s/.stopmotion/trash/", getenv("HOME") );
	mkdir(tmp, 0755);
	sprintf( tmp, "%s/.stopmotion/packer/", getenv("HOME") );
	mkdir(tmp, 0755);
}


void recover(DomainFacade *facadePtr)
{
	char tmp[256] = {0};
	sprintf( tmp, "%s/.stopmotion/packer/", getenv("HOME") );
	
	DIR *dp = opendir(tmp);
	if (dp) {
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			stat(ep->d_name, &st);
			// is a directory and not a '.' or '..' directory
			if ( S_ISDIR(st.st_mode) != 0 && ep->d_name[0] != '.') {
				PreferencesTool *pref = PreferencesTool::get();
				const char *mostRecent = pref->getPreference("mostRecent", "");
				if ( strstr(mostRecent, ep->d_name) != NULL && 
							access(mostRecent, F_OK) == 0) {
					facadePtr->openProject(mostRecent);
				}
				break;
			}
		}
		closedir(dp);
	}
	
	sprintf( tmp, "%s/.stopmotion/tmp", getenv("HOME") );
	dp = opendir(tmp);
	if (dp) {
		vector<char*> frames;
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			char *fileName = new char[256];
			sprintf(fileName, "%s/%s", tmp, ep->d_name);
			stat(fileName, &st);
			// is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0) {
				frames.push_back(fileName);
			}
			else {
				delete [] fileName;
			}
			fileName = NULL;
			
		}
		closedir(dp);
		
		vector<char*>(frames).swap(frames);
		facadePtr->addFrames(frames);

		unsigned int numElem = frames.size();
		for (unsigned int i = 0; i < numElem; i++) {
			delete [] frames[i];
		}
	}
}



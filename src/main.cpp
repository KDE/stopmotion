#include "config.h"
#include "src/domain/domainfacade.h"

#ifdef QTGUI
#include "presentation/frontends/qtfrontend/qtfrontend.h"
#endif

#include "presentation/frontends/nonguifrontend/nonguifrontend.h"
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


struct AudioFile {
	unsigned int belongsTo;
	char filename[256];
};

bool init(Frontend *frontend, DomainFacade *facadePtr);
void cleanup();
bool isRecoveryMode();
void createDirectories();
void recover(DomainFacade *facadePtr);


int main(int argc, char **argv) 
{
	// returns a pointer that is allocated with new
	DomainFacade *facadePtr = DomainFacade::getFacade();
	
	atexit(cleanup);
	int ret = 0;
	
	// if program is started with --nonGUI
	if ( argc > 1 && strcmp(argv[1], "--nonGUI") == 0 ) {
		NonGUIFrontend *nonGUIFrontend = new NonGUIFrontend(facadePtr);
		facadePtr->registerFrontend(nonGUIFrontend);
		init(nonGUIFrontend, facadePtr);
		ret = nonGUIFrontend->run(argc, argv);
		delete nonGUIFrontend;
		nonGUIFrontend = NULL;
	}
	else {
#ifdef QTGUI
		QtFrontend *qtFrontend = new QtFrontend(argc, argv);
		if (qtFrontend != NULL) {
			facadePtr->registerFrontend(qtFrontend);
			bool hasRecovered = init(qtFrontend, facadePtr);
			if ( hasRecovered == false && argc > 1 && access(argv[1], R_OK) == 0) {
				facadePtr->openProject(argv[1]);
				const char *proFile = facadePtr->getProjectFile();
				if ( proFile != NULL ) {
					PreferencesTool *pref = PreferencesTool::get();
					pref->setPreference("mostRecent", proFile);
				}
			}
			ret = qtFrontend->run(argc, argv);
			delete qtFrontend;
			qtFrontend = NULL;
		}
		else {
			printf("Error: Out of memory!\n");
		}
#endif
	}
	
	delete facadePtr;
	facadePtr = NULL;

	return ret;
}


// creates directories if they already not exist.
bool init(Frontend *frontend, DomainFacade *facadePtr)
{
	if ( isRecoveryMode() ) {
		int ret = frontend->askQuestion(
				"Something caused Stopmotion to exit abnormally\n"
				"last time it was runned. Do you want to recover?");
		// The user wants to recover
		if (ret == 0) {
			recover(facadePtr);
			return true;
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
	return false;
}


// cleanup the directories
void cleanup()
{
	char command[256] = {0};
	snprintf( command, 256, "rm -rf %s/.stopmotion/tmp/", getenv("HOME") );
	system(command);
	snprintf( command, 256, "rm -rf %s/.stopmotion/trash/", getenv("HOME") );
	system(command);
	snprintf( command, 256, "rm -rf %s/.stopmotion/packer/", getenv("HOME") );
	system(command);
}


bool isRecoveryMode()
{
	char tmp[256] = {0};
	snprintf( tmp, 256, "%s/.stopmotion/tmp/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	snprintf( tmp, 256, "%s/.stopmotion/trash/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	snprintf( tmp, 256, "%s/.stopmotion/packer/", getenv("HOME") );
	if ( access(tmp, F_OK) == -1 ) return false;
	
	// Everything is intact and we have to run in recovery mode
	return true;
}


void createDirectories()
{
	char tmp[256] = {0};
	snprintf( tmp, 256, "%s/.stopmotion/", getenv("HOME") );
	
	// is ~/.stopmotion doesn't exist
	if ( access(tmp, F_OK) == -1 ) {
		mkdir(tmp, 0755);
	}
	
	snprintf( tmp, 256, "%s/.stopmotion/tmp/", getenv("HOME") );
	mkdir(tmp, 0755);
	snprintf( tmp, 256, "%s/.stopmotion/trash/", getenv("HOME") );
	mkdir(tmp, 0755);
	snprintf( tmp, 256, "%s/.stopmotion/packer/", getenv("HOME") );
	mkdir(tmp, 0755);
}


void recover(DomainFacade *facadePtr)
{
	char tmp[256] = {0};
	snprintf( tmp, 256, "%s/.stopmotion/packer/", getenv("HOME") );
	
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
	
	snprintf( tmp, 256, "%s/.stopmotion/tmp", getenv("HOME") );
	dp = opendir(tmp);
	if (dp) {
		vector<char*> frames;
		vector<AudioFile> sounds;
		struct dirent *ep;
		struct stat st;

		while ( (ep = readdir(dp)) ) {
			char *fileName = new char[256];
			snprintf(fileName, 256, "%s/%s", tmp, ep->d_name);
			stat(fileName, &st);
			// Is a regular file, not a directory
			if ( S_ISREG(st.st_mode) != 0) {
				// Image file
				if ( strstr(fileName, "snd") == NULL ) {
					frames.push_back(fileName);
				}
				// Sound file
				else {
					char buf[256] = {0};
					char index[256] = {0};
					
					char *firstDelim = strchr(fileName, '_');
					strcpy(buf, firstDelim + 1);
					char *secondDelim = strchr(buf, '_');
					strncpy(index, buf, strlen(buf) - strlen(secondDelim));
					
					AudioFile af;
					af.belongsTo = (unsigned int)atoi(index);
					strcpy(af.filename, fileName);
					sounds.push_back(af);	
				}
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
		
		numElem = sounds.size();
		for (unsigned int j = 0; j < numElem; j++) {
			facadePtr->addSound(sounds[j].belongsTo, sounds[j].filename);
		}
	}
}



/***************************************************************************
 *   Copyright (C) 2005-2024 by Linuxstopmotion contributors;              *
 *   see the AUTHORS file for details.                                     *
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

#include "projectserializer.h"

#include "src/foundation/logger.h"
#include "src/domain/filenamevisitor.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/sound.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/workspacefile.h"
#include "src/presentation/frontends/frontend.h"

#include <libxml/tree.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>
#include <cstring>
#include <errno.h>
#include <iosfwd>
#include <map>
#include <string>
#include <set>

FileException::FileException(const char* functionName, int errorno) {
	snprintf(buffer, sizeof(buffer), "%s(): %s", functionName,
			strerror(errorno));
}

const char* FileException::what() const throw() {
	return buffer;
}

class ProjectFileCreationException : public FileException {
public:
	ProjectFileCreationException(const char* functionName, int errorno)
			: FileException(functionName, errorno) {
	}
};

class ProjectFileReadException : public FileException {
public:
	ProjectFileReadException(const char* functionName, int errorno)
			: FileException(functionName, errorno) {
	}
};

ProjectSerializer::ProjectSerializer() : projectFile(0) {
	LIBXML_TEST_VERSION;
}

ProjectSerializer::~ProjectSerializer() {
	resetProjectFile();
}

class TarFileRead {
	struct archive* tar = archive_read_new();
	struct archive_entry *entry;
public:
	TarFileRead(const char* filename) {
		if (archive_read_support_format_tar(tar) != ARCHIVE_OK) {
			throw ProjectFileReadException("archive_read_support_format_tar", errno);
		}
		if (archive_read_open_filename(tar, filename, 4096) != ARCHIVE_OK) {
			throw ProjectFileReadException("archive_read_open_filename", errno);
		}
	}
	~TarFileRead() {
	}
	bool next() {
		int ret = archive_read_next_header(tar, &entry);
		if (ret == ARCHIVE_OK || ret == ARCHIVE_RETRY || ret == ARCHIVE_WARN)
			return true;
		if (ret == ARCHIVE_FATAL)
			throw ProjectFileReadException("archive_read_next_header", errno);
		return false;
	}
	const char *regularFileFilename() const {
		return archive_entry_filetype(entry) == AE_IFREG ? archive_entry_pathname(entry) : 0;
	}
	void extract(std::string& filename) {
		filename.c_str();
		int fd = open(&filename[0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fd < 0)
			throw ProjectFileReadException("open", errno);
		if (archive_read_data_into_fd(tar, fd) < 0) {
			throw ProjectFileReadException("archive_read_data_into_fd", errno);
		}
	}
	void finish() {
		if (archive_read_close(tar) != ARCHIVE_OK)
			throw ProjectFileReadException("archive_read_close", errno);
		if (archive_read_free(tar) != ARCHIVE_OK)
			throw ProjectFileReadException("archive_read_finish", errno);
		tar = NULL;
	}
};

class StoFileRead {
	TarFileRead tar;
	static bool isProjectFile(const char* filename) {
		if (filename) {
			const char* slash1 = strchr(filename, '/');
			if (slash1) {
				++slash1;
				if (!strchr(slash1, '/')) {
					const char* dot = strrchr(slash1, '.');
					if (dot && strcmp(dot, ".dat") == 0) {
						return true;
					}
				}
			}
		}
		return false;
	}
	// returns the basename portion of the argument if it is an image or sound,
	// null otherwise
	static const char* imageOrSoundFile(const char* filename) {
		static const char images[] = "images";
		static const char sounds[] = "sounds";
		if (filename) {
			const char* slash1 = strchr(filename, '/');
			if (slash1) {
				++slash1;
				const char* slash2 = strchr(slash1, '/');
				if (slash2) {
					std::ptrdiff_t dirLen = slash2 - slash1;
					++slash2;
					if ((strncmp(images, slash1, dirLen) == 0
							|| strncmp(sounds, slash1, dirLen))
							&& strchr(slash2, '/') == 0) {
						return slash2;
					}
				}
			}
		}
		return 0;
	}
public:
	StoFileRead(const char* filename) : tar(filename) {
	}
	~StoFileRead() {
	}
	void unpack() {
		while (tar.next()) {
			const char* fn = tar.regularFileFilename();
			const char* base = imageOrSoundFile(fn);
			if (base) {
				// image or sound
				WorkspaceFile wf(base);
				std::string path = wf.path();
				tar.extract(path);
			} else if (isProjectFile(fn)) {
				WorkspaceFile dat(WorkspaceFile::currentModelFile);
				std::string path = dat.path();
				tar.extract(path);
			} else if (fn) {
				Logger::get().logWarning(
						"Did not know what to do with tarred file %s", fn);
			}
		}
	}
	void finish() {
		tar.finish();
	}
};

void readSto(const char* filename) {
	StoFileRead sto(filename);
	sto.unpack();
	sto.finish();
}

// appends .sto to filename if appropriate to make a project filename
void ProjectSerializer::setProjectFile(const char *filename) {
	static const char appendix[] = ".sto";
	const size_t len = strlen(filename);
	char* newProjectFile = new char[len + sizeof(appendix)];
	strcpy(newProjectFile, filename);
	const char *dotPtr = strrchr(filename, '.');
	if (dotPtr == NULL || strcmp(dotPtr, ".sto") != 0) {
		strcat(newProjectFile, ".sto");
	}
	delete[] projectFile;
	projectFile = newProjectFile;
}

bool ProjectSerializer::openDat(std::vector<Scene*>& out, const char* filename) {
	xmlDocPtr doc = xmlReadFile(filename, NULL, 0);
	if (!doc) {
		Logger::get().logWarning("Couldn't load XML file");
		return false;
	}
	xmlNodePtr rootNode = xmlDocGetRootElement(doc);
	out.clear();
	getAttributes(rootNode, out);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return true;
}

std::vector<Scene*> ProjectSerializer::openSto(const char *filename) {
	assert(filename != NULL);
	setProjectFile(filename);
	readSto(projectFile);
	WorkspaceFile dat(WorkspaceFile::currentModelFile);
	std::vector<Scene*> sVect;
	openDat(sVect, dat.path());
	return sVect;
}

class UniqueVisitor: public FileNameVisitor {
	std::set<std::string> visited;
	FileNameVisitor* del;
public:
	UniqueVisitor(FileNameVisitor* delegate) :
			del(delegate) {
	}
	~UniqueVisitor() {
	}
	void visitImage(const char* path) {
		std::string p(path);
		if (visited.find(p) == visited.end()) {
			visited.insert(p);
			del->visitImage(path);
		}
	}
	void visitSound(const char* path) {
		std::string p(path);
		if (visited.find(p) == visited.end()) {
			visited.insert(p);
			del->visitSound(path);
		}
	}
};

std::string getProjectName(const char* filename) {
	const char* nameBegin = strrchr(filename, '/');
	nameBegin = !nameBegin? filename : nameBegin + 1;
	const char* nameEnd = strrchr(filename, '.');
	if (!nameEnd || nameEnd < nameBegin)
		nameEnd = filename + strlen(filename);
	return std::string(nameBegin, nameEnd);
}

class TarPath {
	std::string path;
	std::string::size_type length;
public:
	TarPath(const std::string& rootDir, const char* ext)
			: path(rootDir), length(0) {
		path.append(ext);
		length = path.length();
	}
	/**
	 * Returns the argument prefixed with the rootDir and ext supplied
	 * to the constructor. The result is valid until the next call of getPath
	 * or this object is destroyed.
	 */
	const char* getPath(const char* fname) {
		path.resize(length);
		path.append(fname);
		return path.c_str();
	}
};

class TarFileWrite {
	struct archive* tar = archive_write_new();
public:
	TarFileWrite(const char* tarFilename) {
		if (archive_write_set_format_ustar(tar) != ARCHIVE_OK) {
			throw ProjectFileCreationException("archive_write_set_format_ustar", errno);
		}
		if (archive_write_open_filename(tar, tarFilename) != ARCHIVE_OK) {
			throw ProjectFileCreationException("archive_write_open_filename", errno);
		}
	}
	~TarFileWrite() {
	}
	void add(const char* realPath, const char* storedPath) {
		char buff[4096];
		int fd;
		int len;
		struct stat st;
		struct archive_entry *entry = archive_entry_new();
		if (stat(realPath, &st) < 0)
			throw ProjectFileCreationException("stat", errno);
		archive_entry_copy_stat(entry, &st);
		if (storedPath)
			archive_entry_set_pathname(entry, storedPath);
		if (archive_write_header(tar, entry) != ARCHIVE_OK)
			throw ProjectFileCreationException("archive_write_header", errno);

		fd = open(realPath, O_RDONLY);
		if (fd < 0)
			throw ProjectFileCreationException("open", errno);
		len = read(fd, buff, sizeof(buff));
		while (len > 0) {
			if (archive_write_data(tar, buff, len) < 0)
				throw ProjectFileCreationException("archive_write_data", errno);
			len = read(fd, buff, sizeof(buff));
		}
		close(fd);
		archive_entry_free(entry);
	}
	void eof() {
		if (archive_write_close(tar) != ARCHIVE_OK)
			throw ProjectFileCreationException("archive_write_close", errno);
		if (archive_write_free(tar) != ARCHIVE_OK)
			throw ProjectFileCreationException("archive_write_free", errno);
		tar = NULL;
	}
};

class StoFileWrite {
	TarFileWrite tar;
	std::string name;
	TarPath imagePath;
	TarPath soundPath;
	std::set<std::string> alreadyPacked;
public:
	StoFileWrite(const char* stoFilename)
			: tar(stoFilename),
			  name(getProjectName(stoFilename)),
			  imagePath(name, "/images/"),
			  soundPath(name, "/sounds/") {
	}
	void addProjectFile(const char* projectFilename) {
		std::string projectFileName = name;
		projectFileName.append("/");
		projectFileName.append(name);
		projectFileName.append(".dat");
		tar.add(projectFilename, projectFileName.c_str());
	}
	void addImage(const char* realName, const char* baseName) {
		std::string bn(baseName);
		if (alreadyPacked.insert(bn).second)
			tar.add(realName, imagePath.getPath(baseName));
	}
	void addSound(const char* realName, const char* baseName) {
		std::string bn(baseName);
		if (alreadyPacked.insert(bn).second)
			tar.add(realName, soundPath.getPath(baseName));
	}
	void finish() {
		tar.eof();
	}
};

void writeSto(const char* filename, const char* xmlProjectFile,
			const AnimationImpl& anim) {
	StoFileWrite sto(filename);
	sto.addProjectFile(xmlProjectFile);
	int sceneCount = anim.sceneCount();
	for (int i = 0; i != sceneCount; ++i) {
		const Scene* scene = anim.getScene(i);
		int frameCount = scene->getSize();
		for (int j = 0; j != frameCount; ++j) {
			const Frame* frame = scene->getFrame(j);
			sto.addImage(frame->getImagePath(), frame->getBasename());
			int soundCount = frame->soundCount();
			for (int k = 0; k != soundCount; ++k) {
				const Sound* sound = frame->getSound(k);
				sto.addSound(sound->getSoundPath(), sound->getBasename());
			}
		}
	}
	sto.finish();
}

/**
 * Save the current file, putting the current state into the new model file.
 * The caller is responsible for clearing the command log and then renaming
 * the new model file to be the current model file.
 * @param filename The filename to save.
 * @param anim The animation to save.
 * @param frontend The UI for reporting progress and errors.
 */
void ProjectSerializer::save(const char *filename,
		const AnimationImpl& anim, Frontend *frontend) {
	assert(filename != NULL);
	assert(frontend != NULL);

	setProjectFile(filename);

	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlCreateIntSubset(doc, BAD_CAST "smil",
			BAD_CAST "-//W3C//DTD SMIL 2.0//EN",
			BAD_CAST "http://www.w3.org/2001/SMIL20/SMIL20.dtd");

	xmlNodePtr rootNode = xmlNewNode(NULL, BAD_CAST "smil");
	xmlNewProp(rootNode, BAD_CAST "xmlns",
			BAD_CAST "http://www.w3.org/2001/SMIL20/Language");
	xmlNewProp(rootNode, BAD_CAST "xml:lang", BAD_CAST "en");
	xmlNewProp(rootNode, BAD_CAST "title", BAD_CAST "Stopmotion");
	xmlDocSetRootElement(doc, rootNode);

	setAttributes(rootNode, anim, frontend);

	WorkspaceFile newDat(WorkspaceFile::newModelFile);
	saveDOMToFile(doc, newDat.path());

	xmlFreeDoc(doc);
	xmlCleanupParser();

	// Write out new.dat file. The recovery system will ignore it until...
	writeSto(projectFile, newDat.path(), anim);
	int numElem = anim.sceneCount();
	frontend->updateProgress(numElem);
	frontend->hideProgress();
}

void ProjectSerializer::setAttributes(xmlNodePtr rootNode,
		const AnimationImpl& anim, Frontend *frontend) {
	xmlNodePtr node = NULL;
	const Frame *frame = NULL;
	const Sound *sound = NULL;

	xmlNodePtr scenes = xmlNewChild(rootNode, NULL, BAD_CAST "scenes", NULL);

	int numScenes = anim.sceneCount();
	frontend->showProgress(Frontend::savingScenesToDisk, numScenes);
	for (int i = 0; i < numScenes; ++i) {
		frontend->updateProgress(i);
		// Scenes
		node = xmlNewChild(scenes, NULL, BAD_CAST "seq", NULL);

		// Images
		xmlNodePtr images = xmlNewChild(node, NULL, BAD_CAST "images", NULL);
		const Scene* scene = anim.getScene(i);
		int numFrames = scene->getSize();
		for (int j = 0; j < numFrames; ++j) {
			frame = scene->getFrame(j);
			const char *filename = frame->getBasename();
			node = xmlNewChild(images, NULL, BAD_CAST "img", NULL);
			xmlNewProp(node, BAD_CAST "src", BAD_CAST filename);

			// Sounds
			int numSounds = frame->soundCount();
			if (numSounds > 0) {
				xmlNodePtr sounds = xmlNewChild(node, NULL, BAD_CAST "sounds",
						NULL);
				for (int k = 0; k < numSounds; ++k) {
					sound = frame->getSound(k);
					filename = sound->getBasename();
					node = xmlNewChild(sounds, NULL, BAD_CAST "audio", NULL);
					xmlNewProp(node, BAD_CAST "src", BAD_CAST filename);
					xmlNewProp(node, BAD_CAST "alt",
							BAD_CAST frame->getSoundName(i));
				}
			}
		}
	}
}

void ProjectSerializer::getAttributes(xmlNodePtr node,
		std::vector<Scene*>& sVect) {
	xmlNodePtr currNode = NULL;
	for (currNode = node; currNode; currNode = currNode->next) {
		if (currNode->type == XML_ELEMENT_NODE) {
			char *nodeName = (char*) currNode->name;
			// We either have a image node or a sound node
			if (strcmp(nodeName, "img") == 0
					|| strcmp(nodeName, "audio") == 0) {
				char *filename = (char*) xmlGetProp(currNode, BAD_CAST "src");
				if (filename != NULL) {
					// The node is a image node
					if (strcmp(nodeName, "img") == 0) {
						WorkspaceFile wf(filename);
						Frame *f = new Frame(wf);
						Scene *s = sVect.back();
						s->addSavedFrame(f);
					}
					// The node is a sound node
					else {
						Scene *s = sVect.back();
						int frameNum = s->getSize() - 1;
						WorkspaceFile wf(filename);
						int soundNum = s->soundCount(frameNum);
						s->newSound(frameNum, wf);
						char *soundName = (char*) xmlGetProp(currNode,
								BAD_CAST "alt");
						if (soundName != NULL) {
							s->setSoundName(frameNum, soundNum, soundName);
							xmlFree((xmlChar*) soundName);
						}
					}
					xmlFree((xmlChar*) filename);
				}
			}
			// The node is a scene node
			else if (strcmp((char*) currNode->name, "seq") == 0) {
				Scene *s = new Scene();
				sVect.push_back(s);
			}
		}
		getAttributes(currNode->children, sVect);
	}
}

void ProjectSerializer::saveDOMToFile(xmlDocPtr doc, const char* filename) {
	int ret = xmlSaveFormatFile(filename, doc, 1);
	if (ret == -1) {
		throw ProjectFileCreationException("xmlSaveFormatFile", errno);
	}
}

const char* ProjectSerializer::getProjectFile() {
	return projectFile;
}

void ProjectSerializer::resetProjectFile(const char* filename) {
	char* copy = 0;
	if (filename) {
		size_t len = strlen(filename);
		copy = new char[len + 1];
		strncpy(copy, filename, len + 1);
	}
	delete[] projectFile;
	projectFile = copy;
}

void ProjectSerializer::resetProjectFile() {
	resetProjectFile(0);
}

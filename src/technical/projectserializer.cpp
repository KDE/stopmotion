/*
 * SPDX-FileCopyrightText: 2005-2024 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "projectserializer.h"

#include "src/foundation/logger.h"
#include "src/domain/filenamevisitor.h"
#include "src/domain/animation/scene.h"
#include "src/domain/animation/frame.h"
#include "src/domain/animation/sound.h"
#include "src/domain/animation/animationimpl.h"
#include "src/domain/animation/workspacefile.h"
#include "src/presentation/frontends/frontend.h"

#include <KArchiveDirectory>
#include <KArchiveFile>
#include <KTar>

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QString>
#include <QXmlStreamWriter>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <set>

FileException::FileException(const char* functionName, int errorno) {
	snprintf(buffer, sizeof(buffer), "%s(): %s", functionName,
			strerror(errorno));
}

FileException::FileException(const char* functionName, const char* message) {
	snprintf(buffer, sizeof(buffer), "%s(): %s", functionName, message);
}

const char* FileException::what() const noexcept {
	return buffer;
}

class ProjectFileCreationException : public FileException {
public:
	ProjectFileCreationException(const char* functionName, int errorno)
			: FileException(functionName, errorno) {
	}
	ProjectFileCreationException(const char* functionName,
			const QString& message)
			: FileException(functionName, message.toLocal8Bit().constData()) {
	}
};

class ProjectFileReadException : public FileException {
public:
	ProjectFileReadException(const char* functionName, const QString& message)
			: FileException(functionName, message.toLocal8Bit().constData()) {
	}
};

namespace {
// Project files are uncompressed tar archives. Pass the type explicitly, as
// KTar cannot guess it from the .sto extension.
const QString tarMimeType = QStringLiteral("application/x-tar");
}

ProjectSerializer::ProjectSerializer() : projectFile(nullptr) {
}

ProjectSerializer::~ProjectSerializer() {
	resetProjectFile();
}

class TarFileRead {
	KTar tar;
	template<typename F>
	static void forEachFile(const KArchiveDirectory* dir,
			const QString& prefix, F& f) {
		const QStringList names = dir->entries();
		for (const QString& name : names) {
			const KArchiveEntry* entry = dir->entry(name);
			const QString path = prefix + name;
			if (entry->isDirectory()) {
				forEachFile(static_cast<const KArchiveDirectory*>(entry),
						path + QLatin1Char('/'), f);
			} else if (entry->isFile() && entry->symLinkTarget().isEmpty()) {
				f(path, static_cast<const KArchiveFile*>(entry));
			}
		}
	}
public:
	TarFileRead(const char* filename)
			: tar(QString::fromLocal8Bit(filename), tarMimeType) {
		if (!tar.open(QIODevice::ReadOnly)) {
			// KTar does not say why it could not open the file itself
			QFile file(QString::fromLocal8Bit(filename));
			if (!file.open(QIODevice::ReadOnly))
				throw ProjectFileReadException("QFile::open", file.errorString());
			throw ProjectFileReadException("KTar::open", tar.errorString());
		}
	}
	/**
	 * Calls f(path, file) for each regular file in the archive, with its path
	 * relative to the root of the archive.
	 */
	template<typename F>
	void forEachFile(F f) {
		forEachFile(tar.directory(), QString(), f);
	}
	static void extract(const KArchiveFile* file, const std::string& filename) {
		std::unique_ptr<QIODevice> in(file->createDevice());
		if (!in)
			throw ProjectFileReadException("KArchiveFile::createDevice",
					QStringLiteral("cannot read %1").arg(file->name()));
		QFile out(QString::fromLocal8Bit(filename.c_str()));
		if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
			throw ProjectFileReadException("QFile::open", out.errorString());
		char buffer[65536];
		qint64 n;
		qint64 total = 0;
		while (0 < (n = in->read(buffer, sizeof(buffer)))) {
			if (out.write(buffer, n) != n)
				throw ProjectFileReadException("QFile::write", out.errorString());
			total += n;
		}
		if (n < 0)
			throw ProjectFileReadException("QIODevice::read", in->errorString());
		// KTar opens truncated archives without complaint
		if (total != file->size())
			throw ProjectFileReadException("KArchiveFile::createDevice",
					QStringLiteral("%1 is truncated").arg(file->name()));
		out.close();
		if (out.error() != QFileDevice::NoError)
			throw ProjectFileReadException("QFile::close", out.errorString());
	}
	void finish() {
		if (!tar.close())
			throw ProjectFileReadException("KTar::close", tar.errorString());
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
					if (((dirLen == sizeof(images) - 1
								&& strncmp(images, slash1, dirLen) == 0)
							|| (dirLen == sizeof(sounds) - 1
								&& strncmp(sounds, slash1, dirLen) == 0))
							&& strchr(slash2, '/') == nullptr) {
						return slash2;
					}
				}
			}
		}
		return nullptr;
	}
public:
	StoFileRead(const char* filename) : tar(filename) {
	}
	~StoFileRead() {
	}
	void unpack() {
		tar.forEachFile([](const QString& path, const KArchiveFile* file) {
			const QByteArray name = path.toLocal8Bit();
			const char* fn = name.constData();
			const char* base = imageOrSoundFile(fn);
			if (base) {
				// image or sound
				WorkspaceFile wf(base);
				TarFileRead::extract(file, wf.path());
			} else if (isProjectFile(fn)) {
				WorkspaceFile dat(WorkspaceFile::currentModelFile);
				TarFileRead::extract(file, dat.path());
			} else {
				Logger::get().logWarning(
						"Did not know what to do with tarred file %s", fn);
			}
		});
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
	if (dotPtr == nullptr || strcmp(dotPtr, ".sto") != 0) {
		strcat(newProjectFile, ".sto");
	}
	delete[] projectFile;
	projectFile = newProjectFile;
}

bool ProjectSerializer::openDat(std::vector<Scene*>& out, const char* filename) {
	QFile file(QString::fromLocal8Bit(filename));
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file)) {
		Logger::get().logWarning("Couldn't load XML file");
		return false;
	}
	out.clear();
	try {
		getAttributes(doc.documentElement(), out);
	} catch (...) {
		for (Scene* s : out) {
			delete s;
		}
		out.clear();
		throw;
	}
	return true;
}

std::vector<Scene*> ProjectSerializer::openSto(const char *filename) {
	assert(filename != nullptr);
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
	~UniqueVisitor() override {
	}
	void visitImage(const char* path) override {
		std::string p(path);
		if (visited.insert(p).second) {
			del->visitImage(path);
		}
	}
	void visitSound(const char* path) override {
		std::string p(path);
		if (visited.insert(p).second) {
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
	QString filename;
	KTar tar;
public:
	TarFileWrite(const char* tarFilename)
			: filename(QString::fromLocal8Bit(tarFilename)),
			  tar(filename, tarMimeType) {
		if (!tar.open(QIODevice::WriteOnly))
			throw ProjectFileCreationException("KTar::open", tar.errorString());
	}
	void add(const char* realPath, const char* storedPath) {
		if (!tar.addLocalFile(QString::fromLocal8Bit(realPath),
				QString::fromLocal8Bit(storedPath)))
			throw ProjectFileCreationException("KTar::addLocalFile",
					tar.errorString());
	}
	void eof() {
		if (!tar.close())
			throw ProjectFileCreationException("KTar::close", tar.errorString());
		// A tar archive ends with two zero-filled blocks, which KTar does not
		// write. Add them, as earlier versions of Stopmotion did (#16).
		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append)
				|| file.write(QByteArray(2 * 512, '\0')) != 2 * 512
				|| !file.flush())
			throw ProjectFileCreationException("QFile::write", file.errorString());
		file.close();
		if (file.error() != QFileDevice::NoError)
			throw ProjectFileCreationException("QFile::close", file.errorString());
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
	assert(filename != nullptr);
	assert(frontend != nullptr);

	setProjectFile(filename);

	WorkspaceFile newDat(WorkspaceFile::newModelFile);
	writeDat(newDat.path(), anim, frontend);

	// Write out new.dat file. The recovery system will ignore it until...
	writeSto(projectFile, newDat.path(), anim);
	int numElem = anim.sceneCount();
	frontend->updateProgress(numElem);
	frontend->hideProgress();
}

void ProjectSerializer::writeDat(const char* filename,
		const AnimationImpl& anim, Frontend *frontend) {
	QFile file(QString::fromLocal8Bit(filename));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		throw ProjectFileCreationException("QFile::open", file.errorString());
	QXmlStreamWriter writer(&file);
	writer.setAutoFormatting(true);
	writer.setAutoFormattingIndent(2);
	writer.writeStartDocument();
	writer.writeDTD(QStringLiteral("<!DOCTYPE smil PUBLIC \"-//W3C//DTD SMIL 2.0//EN\" "
			"\"http://www.w3.org/2001/SMIL20/SMIL20.dtd\">"));
	writer.writeStartElement(QStringLiteral("smil"));
	writer.writeDefaultNamespace(
			QStringLiteral("http://www.w3.org/2001/SMIL20/Language"));
	writer.writeAttribute(QStringLiteral("xml:lang"), QStringLiteral("en"));
	writer.writeAttribute(QStringLiteral("title"), QStringLiteral("Stopmotion"));

	writer.writeStartElement(QStringLiteral("scenes"));
	int numScenes = anim.sceneCount();
	frontend->showProgress(Frontend::savingScenesToDisk, numScenes);
	for (int i = 0; i < numScenes; ++i) {
		frontend->updateProgress(i);
		writer.writeStartElement(QStringLiteral("seq"));
		writer.writeStartElement(QStringLiteral("images"));
		const Scene* scene = anim.getScene(i);
		int numFrames = scene->getSize();
		for (int j = 0; j < numFrames; ++j) {
			const Frame* frame = scene->getFrame(j);
			writer.writeStartElement(QStringLiteral("img"));
			writer.writeAttribute(QStringLiteral("src"),
					QString::fromUtf8(frame->getBasename()));
			int numSounds = frame->soundCount();
			if (numSounds > 0) {
				writer.writeStartElement(QStringLiteral("sounds"));
				for (int k = 0; k < numSounds; ++k) {
					const Sound* sound = frame->getSound(k);
					writer.writeStartElement(QStringLiteral("audio"));
					writer.writeAttribute(QStringLiteral("src"),
							QString::fromUtf8(sound->getBasename()));
					writer.writeAttribute(QStringLiteral("alt"),
							QString::fromUtf8(sound->getName()));
					writer.writeEndElement(); // audio
				}
				writer.writeEndElement(); // sounds
			}
			writer.writeEndElement(); // img
		}
		writer.writeEndElement(); // images
		writer.writeEndElement(); // seq
	}
	writer.writeEndElement(); // scenes
	writer.writeEndElement(); // smil
	writer.writeEndDocument();

	if (writer.hasError())
		throw ProjectFileCreationException("QXmlStreamWriter", file.errorString());
	file.close();
	if (file.error() != QFileDevice::NoError)
		throw ProjectFileCreationException("QFile::close", file.errorString());
}

void ProjectSerializer::getAttributes(const QDomNode& node,
		std::vector<Scene*>& sVect) {
	for (QDomNode currNode = node; !currNode.isNull();
			currNode = currNode.nextSibling()) {
		const QDomElement element = currNode.toElement();
		if (!element.isNull()) {
			const QString nodeName = element.tagName();
			// We either have a image node or a sound node
			if (nodeName == QLatin1String("img")
					|| nodeName == QLatin1String("audio")) {
				if (!element.hasAttribute(QStringLiteral("src"))) {
					// nothing to load
				} else if (sVect.empty()) {
					Logger::get().logWarning("Ignoring %s outside of any scene",
							element.attribute(QStringLiteral("src")).toUtf8().constData());
				} else {
					const QByteArray filename =
							element.attribute(QStringLiteral("src")).toUtf8();
					WorkspaceFile wf(filename.constData());
					// e.g. a truncated project file
					if (!QFile::exists(QString::fromLocal8Bit(wf.path()))) {
						throw ProjectFileReadException("openDat",
								QStringLiteral("%1 is missing from the project")
								.arg(QString::fromUtf8(filename)));
					}
					// The node is a image node
					if (nodeName == QLatin1String("img")) {
						std::unique_ptr<Frame> f(new Frame(wf));
						Scene *s = sVect.back();
						s->addSavedFrame(f.get());
						f.release();
					}
					// The node is a sound node
					else {
						Scene *s = sVect.back();
						int frameNum = s->getSize() - 1;
						int soundNum = s->soundCount(frameNum);
						s->newSound(frameNum, wf);
						if (element.hasAttribute(QStringLiteral("alt"))) {
							const QByteArray soundName =
									element.attribute(QStringLiteral("alt")).toUtf8();
							// setSoundName takes ownership of a new[] string
							// and returns ownership of the old name
							size_t size = soundName.size() + 1;
							std::unique_ptr<char[]> name(new char[size]);
							memcpy(name.get(), soundName.constData(), size);
							delete[] s->setSoundName(frameNum, soundNum,
									name.get());
							name.release();
						}
					}
				}
			}
			// The node is a scene node
			else if (nodeName == QLatin1String("seq")) {
				Scene *s = new Scene();
				sVect.push_back(s);
			}
		}
		getAttributes(currNode.firstChild(), sVect);
	}
}

const char* ProjectSerializer::getProjectFile() {
	return projectFile;
}

void ProjectSerializer::resetProjectFile(const char* filename) {
	char* copy = nullptr;
	if (filename) {
		size_t len = strlen(filename);
		copy = new char[len + 1];
		strncpy(copy, filename, len + 1);
	}
	delete[] projectFile;
	projectFile = copy;
}

void ProjectSerializer::resetProjectFile() {
	resetProjectFile(nullptr);
}

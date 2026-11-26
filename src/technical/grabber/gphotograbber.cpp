/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "gphotograbber.h"
#include "stopmotion_debug.h"

#include <QByteArray>
#include <QFile>
#include <QImage>

#include <gphoto2/gphoto2.h>

namespace {
/** Owns a CameraFile. */
class File {
	CameraFile* file = nullptr;
public:
	File() {
		gp_file_new(&file);
	}
	~File() {
		if (file)
			gp_file_unref(file);
	}
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	CameraFile* get() const {
		return file;
	}
	QByteArray data() const {
		const char* d = nullptr;
		unsigned long size = 0;
		if (!file || gp_file_get_data_and_size(file, &d, &size) != GP_OK)
			return QByteArray();
		return QByteArray(d, static_cast<qsizetype>(size));
	}
};
}

GPhotoGrabber::GPhotoGrabber(QObject* parent) : CameraGrabber(parent) {
}

GPhotoGrabber::~GPhotoGrabber() {
	stopping = true;
	if (worker.joinable())
		worker.join();
	if (camera) {
		gp_camera_exit(camera, context);
		gp_camera_unref(camera);
	}
	if (context)
		gp_context_unref(context);
}

QString GPhotoGrabber::errorText(int result) const {
	return QString::fromUtf8(gp_result_as_string(result));
}

bool GPhotoGrabber::start(const QByteArray&) {
	context = gp_context_new();
	int result = gp_camera_new(&camera);
	if (result == GP_OK)
		result = gp_camera_init(camera, context);
	if (result != GP_OK) {
		detail = errorText(result);
		if (camera) {
			gp_camera_unref(camera);
			camera = nullptr;
		}
		return false;
	}
	worker = std::thread(&GPhotoGrabber::run, this);
	return true;
}

bool GPhotoGrabber::storeImage(const QString& path) {
	std::lock_guard<std::mutex> lock(mutex);
	switch (captureState) {
	case idle:
		capturePath = path;
		captureState = requested;
		return false;
	case stored:
		if (path != capturePath)
			return false;
		captureState = idle;
		return true;
	default:
		return false;
	}
}

void GPhotoGrabber::run() {
	while (!stopping) {
		QString path;
		{
			std::lock_guard<std::mutex> lock(mutex);
			if (captureState == requested) {
				captureState = capturing;
				path = capturePath;
			}
		}
		if (!path.isEmpty()) {
			const bool ok = capture(path);
			std::lock_guard<std::mutex> lock(mutex);
			captureState = ok? stored : idle;
			if (!ok)
				return;
		} else if (!preview()) {
			return;
		}
	}
}

bool GPhotoGrabber::preview() {
	File file;
	const int result = gp_camera_capture_preview(camera, file.get(), context);
	if (result != GP_OK) {
		Q_EMIT failed(errorText(result));
		return false;
	}
	const QImage image = QImage::fromData(file.data());
	if (!image.isNull())
		Q_EMIT imageChanged(image);
	return true;
}

bool GPhotoGrabber::capture(const QString& path) {
	qCDebug(STOPMOTION_LOG, "Capturing with gphoto2");
	CameraFilePath onCamera;
	int result = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &onCamera, context);
	File file;
	if (result == GP_OK) {
		result = gp_camera_file_get(camera, onCamera.folder, onCamera.name,
				GP_FILE_TYPE_NORMAL, file.get(), context);
	}
	if (result != GP_OK) {
		Q_EMIT failed(errorText(result));
		return false;
	}
	// the photo is stored as it is, unless it is no JPEG
	const QByteArray data = file.data();
	QImage image;
	if (!image.loadFromData(data)) {
		Q_EMIT failed(tr("The photo could not be read. "
				"Please set the camera to store JPEG images."));
		return false;
	}
	QFile out(path);
	const bool written = data.startsWith("\xff\xd8")
			? out.open(QIODevice::WriteOnly) && out.write(data) == data.size()
			: image.save(path, "JPEG", 95);
	if (!written)
		Q_EMIT failed(tr("The photo could not be stored."));
	return written;
}

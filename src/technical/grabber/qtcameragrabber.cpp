/*
 * SPDX-FileCopyrightText: 2026 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qtcameragrabber.h"
#include "stopmotion_debug.h"

#include <QByteArray>
#include <QCamera>
#include <QCameraFormat>
#include <QList>
#include <QString>
#include <QVideoFrame>

#include <algorithm>

namespace {
/** The format with the most pixels and, among those, the highest frame rate. */
QCameraFormat bestFormat(const QCameraDevice& device) {
	const QList<QCameraFormat> formats = device.videoFormats();
	const auto best = std::max_element(formats.begin(), formats.end(),
			[](const QCameraFormat& a, const QCameraFormat& b) {
		const qint64 pixelsA = qint64(a.resolution().width()) * a.resolution().height();
		const qint64 pixelsB = qint64(b.resolution().width()) * b.resolution().height();
		if (pixelsA != pixelsB)
			return pixelsA < pixelsB;
		return a.maxFrameRate() < b.maxFrameRate();
	});
	return best == formats.end()? QCameraFormat() : *best;
}
}

QtCameraGrabber::QtCameraGrabber(QObject* parent) : QObject(parent) {
	connect(&sink, &QVideoSink::videoFrameChanged,
			this, &QtCameraGrabber::frameChanged);
	connect(&mediaDevices, &QMediaDevices::videoInputsChanged,
			this, &QtCameraGrabber::videoInputsChanged);
	session.setVideoSink(&sink);
}

QtCameraGrabber::~QtCameraGrabber() {
	if (camera)
		camera->stop();
}

bool QtCameraGrabber::start(const QByteArray& deviceId) {
	const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
	if (cameras.isEmpty())
		return false;
	const auto chosen = std::find_if(cameras.begin(), cameras.end(),
			[&deviceId](const QCameraDevice& c) { return c.id() == deviceId; });
	device = chosen == cameras.end()?
			QMediaDevices::defaultVideoInput() : *chosen;
	if (device.isNull())
		device = cameras.first();
	qCDebug(STOPMOTION_LOG, "Starting camera %s",
			qPrintable(device.description()));
	camera = std::make_unique<QCamera>(device);
	camera->setCameraFormat(bestFormat(device));
	connect(camera.get(), &QCamera::errorOccurred, this,
			[this](QCamera::Error error, const QString& detail) {
		if (error != QCamera::NoError)
			Q_EMIT failed(detail);
	});
	session.setCamera(camera.get());
	camera->start();
	return true;
}

const QImage& QtCameraGrabber::latestImage() const {
	return image;
}

void QtCameraGrabber::frameChanged(const QVideoFrame& frame) {
	QImage i = frame.toImage();
	if (i.isNull())
		return;
	image = std::move(i);
	Q_EMIT imageChanged(image);
}

void QtCameraGrabber::videoInputsChanged() {
	if (!camera)
		return;
	const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
	if (!cameras.contains(device))
		Q_EMIT failed(QString());
}

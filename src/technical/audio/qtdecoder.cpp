/***************************************************************************
 *   Copyright (C) 2019 by Linuxstopmotion contributors;                   *
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

#include "qtdecoder.h"

#include "qtaudiodriver.h"
#include "src/foundation/logger.h"

#include <QAudioDecoder>
#include <QMutex>

#include <algorithm>
#include <memory>

namespace {
// how much data to grab buffer before allowing any to leave
const long INITIAL_BUFFER = 64 * 1024;
}

class QtAudioDecoder::Buffer {
  QByteArray bytes;
  QMutex mutex;
  long position;
  bool finished;
public:
  Buffer() : position(0), finished(false) {
  }
  int remaining() {
    QMutexLocker lock(&mutex);
    return bytes.size() - position;
  }
  void clear() {
    QMutexLocker lock(&mutex);
    position = 0;
    finished = false;
    bytes.clear();
  }
  void append(const char* data, int count) {
    QMutexLocker lock(&mutex);
    bytes.append(data, count);
  }
  void setFinished() {
    QMutexLocker lock(&mutex);
    finished = true;
  }
  // True when no more data will be received
  bool isFinished() {
    QMutexLocker lock(&mutex);
    return finished;
  }
  int getData(char* data, int maxCount) {
    QMutexLocker lock(&mutex);
    int size = bytes.size();
    if ((position == 0 && size < INITIAL_BUFFER) || size == position) {
      // we will have data, but not yet, so stick in a load of zeroes
      std::fill(data, data + maxCount, '\0');
      return maxCount;
    }
    size -= position;
    int count = std::min(size, maxCount);
    const char* from = bytes.constData() + position;
    std::copy(from, from + count, data);
    position += count;
    return count;
  }
};

QtAudioDecoder::QtAudioDecoder(WorkspaceFile& f)
: decoder(0), buffer(0) {
  std::unique_ptr<Buffer> buf(new Buffer);
  // for some reason, allocating it separately from the QtAudioDecoder
  // makes the test pass. Something to do with GLib error interoperability,
  // or luck maybe.
  decoder = new QAudioDecoder();
  connect(decoder, SIGNAL(bufferReady()), this, SLOT(decodedData()));
  connect(decoder, SIGNAL(stateChanged(QAudioDecoder::State)), this, SLOT(decoderStateChanged(QAudioDecoder::State)));
  file.swap(f);
  buffer = buf.release();
}

QtAudioDecoder::~QtAudioDecoder() {
  delete buffer;
  delete decoder;
}

int QtAudioDecoder::open() {
  decoder->setSourceFilename(file.path());
  QAudioFormat af;
  QtAudioDriver::getAudioFormat(af);
  decoder->setAudioFormat(af);
  QAudioDecoder::Error error = decoder->error();
  if (error != QAudioDecoder::NoError) {
    return -1;
  }
  buffer->clear();
  decoder->start();
  return 0;
}

int QtAudioDecoder::close() {
  return 0;
}

void QtAudioDecoder::reset() {
  decoder->stop();
  open();
}

int QtAudioDecoder::fillBuffer(char *audioBuffer, int numBytes) {
  return buffer->getData(audioBuffer, numBytes);
}

const char* QtAudioDecoder::getSoundPath() const {
  return file.path();
}

const char* QtAudioDecoder::getBasename() const {
  return file.basename();
}

int QtAudioDecoder::bytesAvailable() const {
  return buffer->remaining();
}

bool QtAudioDecoder::isFinished() const {
  return buffer->isFinished();
}

void QtAudioDecoder::decodedData() {
  QAudioBuffer ab = decoder->read();
  Logger::get().logDebug("Reading decoded data: %d", ab.byteCount());
  buffer->append(ab.constData<char>(), ab.byteCount());
}

void QtAudioDecoder::decoderStateChanged(QAudioDecoder::State newState) {
  if (newState == QAudioDecoder::StoppedState) {
    buffer->setFinished();
  }
}

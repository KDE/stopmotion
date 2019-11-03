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

#ifndef QTDECODER_H
#define QTDECODER_H

#include "audioformat.h"

#include "src/domain/animation/workspacefile.h"

#include <QAudioDecoder>

class QtAudioDecoder : public QObject, public AudioFormat {
	Q_OBJECT
  WorkspaceFile file;
  QAudioDecoder* decoder;
	class Buffer;
	Buffer* buffer;
public:
  QtAudioDecoder(WorkspaceFile& f);
  ~QtAudioDecoder();
	int open();
	int close();
	void reset();
	int fillBuffer(char *audioBuffer, int numBytes);
	const char* getSoundPath() const;
	const char* getBasename() const;
	int bytesAvailable() const;
public slots:
	void decodedData();
	void decoderStateChanged(QAudioDecoder::State);
};

#endif

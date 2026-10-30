/*
 * SPDX-FileCopyrightText: 2013 Linuxstopmotion contributors; see the AUTHORS file for details
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ADDALLCOMMMANDS_H_
#define ADDALLCOMMMANDS_H_

namespace Commands {
extern const char* addFrames;
extern const char* removeFrames;
extern const char* moveFrames;
extern const char* setImage;
extern const char* addSound;
extern const char* removeSound;
extern const char* renameSound;
extern const char* addScene;
extern const char* removeScene;
extern const char* moveScene;
}

class Executor;
class AnimationImpl;

Executor* makeAnimationCommandExecutor(AnimationImpl& model);

#endif /* ADDALLCOMMMANDS_H_ */

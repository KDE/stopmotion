/***************************************************************************
 *   Copyright (C) 2013 by Linuxstopmotion contributors;                   *
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

#ifndef TESTUNDO_H_
#define TESTUNDO_H_

#include "hash.h"

#include <string>

class Executor;

/**
 * Provides a way for test code to check the state of the model that is being
 * updated by commands in an Executor.
 */
class ModelTestHelper {
public:
	virtual ~ModelTestHelper() = 0;
	/**
	 * Resets the model owned by an Executor to an empty state.
	 * @param [in,out] e The executor whose model must be reset.
	 */
	virtual void resetModel(Executor& e) = 0;
	/**
	 * Returns a hash of the model. The hash of a model should be different
	 * to the hash of similar models, so that hashes comparing equal very
	 * probably means that the models are the same.
	 * @param [in] The exector that owns the model to be hashed.
	 */
	virtual Hash hashModel(const Executor& e) = 0;
	/**
	 * Returns a text string describing the model.
	 */
	virtual void dumpModel(std::string& out, const Executor& e) = 0;
};

/**
 * Tests that the commands in the executor {@a e} obey the following rules:
 * - Two identical commands executed on identical models must produce
 *    identical models.
 * - The execution of a string of commands followed by their inverses (in
 *    reverse order) must leave the model in an identical state.
 * - Under test conditions, only an out-of-memory exception may escape a
 *    command's execution.
 * - If an exception escapes a command's execution, it must not have affected
 *    the model at all.
 */
void testUndo(Executor& e, ModelTestHelper& helper);

#endif /* TESTUNDO_H_ */

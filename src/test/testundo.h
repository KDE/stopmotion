/***************************************************************************
 *   Copyright (C) 2005-2013 by Linuxstopmotion contributors.              *
 *   see contributors.txt for details                                      *
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

#include <stdint.h>

class Executor;

/**
 * Very simple hashing function by Professor Daniel J Bernstein.
 * @par
 * Use this to calculate hashes for @ref ModelTestHelper::HashModel.
 * @note
 * If your model has some sort of hierarchical structure, don't just add all
 * the leaf parts to the hash as this will fail to hash the structure. Instead,
 * hash each sub-part, then add all these hashes to the final hash.
 */
class Hash {
	uint64_t h;
public:
	Hash();
	void Add(uint64_t n);
	void AddS(int64_t n);
	void Add(const char* string);
	void Add(Hash h);
	bool Equals(const Hash& other) const;
};

bool operator==(const Hash& a, const Hash& b);
bool operator!=(const Hash& a, const Hash& b);

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
	virtual void ResetModel(Executor& e) = 0;
	/**
	 * Returns a hash of the model. The hash of a model should be different
	 * to the hash of similar models, so that hashes comparing equal very
	 * probably means that the models are the same.
	 * @param [in] The exector that owns the model to be hashed.
	 */
	virtual Hash HashModel(const Executor& e) = 0;
};

void TestUndo(Executor& e, ModelTestHelper& helper);

#endif /* TESTUNDO_H_ */

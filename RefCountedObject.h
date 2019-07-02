/*
 * MIT License
 *
 * Copyright (c) 2019 jewmin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef Foundation_RefCountedObject_INCLUDED
#define Foundation_RefCountedObject_INCLUDED

#include "CommonDef.h"

namespace Foundation {
	class RefCounter {
	public:
		RefCounter();
		RefCounter(RefCounter &&) = delete;
		RefCounter(const RefCounter &) = delete;
		RefCounter & operator=(RefCounter &&) = delete;
		RefCounter & operator=(const RefCounter &) = delete;
		~RefCounter();

		int operator++();
		int operator++(int);
		int operator--();
		int operator--(int);
		operator int();

	private:
		mutable std::atomic<int> counter_;
	};

	class RefCountedObject {
	public:
		RefCountedObject();
		RefCountedObject(RefCountedObject &&) = delete;
		RefCountedObject(const RefCountedObject &) = delete;
		RefCountedObject & operator=(RefCountedObject &&) = delete;
		RefCountedObject & operator=(const RefCountedObject &) = delete;

		void Duplicate() const;
		void Release();
		int ReferenceCount() const;

	protected:
		virtual ~RefCountedObject();

	private:
		mutable RefCounter counter_;
	};
}

//*********************************************************************
//RefCounter
//*********************************************************************

inline int Foundation::RefCounter::operator++() {
	return counter_.fetch_add(1, std::memory_order_relaxed) + 1;
}

inline int Foundation::RefCounter::operator++(int) {
	return counter_.fetch_add(1, std::memory_order_relaxed);
}

inline int Foundation::RefCounter::operator--() {
	return counter_.fetch_sub(1, std::memory_order_acquire) - 1;
}

inline int Foundation::RefCounter::operator--(int) {
	return counter_.fetch_sub(1, std::memory_order_acquire);
}

inline Foundation::RefCounter::operator int() {
	return counter_;
}

//*********************************************************************
//RefCountedObject
//*********************************************************************

inline void Foundation::RefCountedObject::Duplicate() const {
	counter_++;
}

inline void Foundation::RefCountedObject::Release() {
	if (--counter_ == 0) {
		delete this;
	}
}

inline int Foundation::RefCountedObject::ReferenceCount() const {
	return counter_;
}

#endif
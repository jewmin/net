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

#ifndef Net_Common_Mutex_INCLUDED
#define Net_Common_Mutex_INCLUDED

#include "Net.h"
#include "NetObject.h"

namespace Net {

template<class M>
class NET_EXTERN_TEMPLATE ScopedLock : public NetObject {
public:
	explicit ScopedLock(M & mutex) : mutex_(mutex) { mutex_.Lock(); }
	virtual ~ScopedLock() { mutex_.Unlock(); }

private:
	ScopedLock() = delete;
	ScopedLock(ScopedLock &&) = delete;
	ScopedLock(const ScopedLock &) = delete;
	ScopedLock & operator=(ScopedLock &&) = delete;
	ScopedLock & operator=(const ScopedLock &) = delete;

private:
	M & mutex_;
};

class NET_EXTERN Mutex : public NetObject {
public:
	typedef ScopedLock<Mutex> ScopedLock;

	Mutex() { uv_mutex_init(&mutex_); }
	virtual ~Mutex() { uv_mutex_destroy(&mutex_); }

	void Lock();
	bool TryLock();
	void Unlock();

private:
	Mutex(Mutex &&) = delete;
	Mutex(const Mutex &) = delete;
	Mutex & operator=(Mutex &&) = delete;
	Mutex & operator=(const Mutex &) = delete;

private:
	uv_mutex_t mutex_;
};

//*********************************************************************
//Mutex
//*********************************************************************

inline void Mutex::Lock() {
	uv_mutex_lock(&mutex_);
}

inline bool Mutex::TryLock() {
	return 0 == uv_mutex_trylock(&mutex_);
}

inline void Mutex::Unlock() {
	uv_mutex_unlock(&mutex_);
}

}

#endif
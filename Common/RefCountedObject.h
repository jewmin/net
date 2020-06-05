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

#ifndef Net_Common_RefCountedObject_INCLUDED
#define Net_Common_RefCountedObject_INCLUDED

#include "Common/NetObject.h"
#include "Common/Logger.h"

namespace Net {

class NET_EXTERN RefCounter : public NetObject {
public:
	RefCounter() : counter_(1) {}
	~RefCounter() {}

	i32 operator++();
	i32 operator++(i32);
	i32 operator--();
	i32 operator--(i32);
	operator int();

private:
	RefCounter(RefCounter &&) = delete;
	RefCounter(const RefCounter &) = delete;
	RefCounter & operator=(RefCounter &&) = delete;
	RefCounter & operator=(const RefCounter &) = delete;

private:
	mutable std::atomic<i32> counter_;
};

class NET_EXTERN RefCountedObject : public NetObject {
public:
	virtual ~RefCountedObject() {}

	void Duplicate() const;
	void Release();
	i32 ReferenceCount() const;

protected:
	RefCountedObject() {}

private:
	RefCountedObject(RefCountedObject &&) = delete;
	RefCountedObject(const RefCountedObject &) = delete;
	RefCountedObject & operator=(RefCountedObject &&) = delete;
	RefCountedObject & operator=(const RefCountedObject &) = delete;

private:
	mutable RefCounter counter_;
};

class StrongRefObject;
class NET_EXTERN WeakReference : public RefCountedObject {
	friend class StrongRefObject;
public:
	virtual ~WeakReference() {}
	
	StrongRefObject * Get() const;

protected:
	WeakReference() : object_(nullptr) {}

private:
	StrongRefObject * object_;
};

class NET_EXTERN StrongRefObject : public NetObject {
public:
	StrongRefObject() : weak_reference_(nullptr) {}
	virtual ~StrongRefObject() { ClearWeakReferences(); }

	WeakReference * WeakRef();

protected:
	void ClearWeakReferences();

private:
	StrongRefObject(StrongRefObject &&) = delete;
	StrongRefObject(const StrongRefObject &) = delete;
	StrongRefObject & operator=(StrongRefObject &&) = delete;
	StrongRefObject & operator=(const StrongRefObject &) = delete;

private:
	WeakReference * weak_reference_;
};

//*********************************************************************
//RefCounter
//*********************************************************************

inline i32 Net::RefCounter::operator++() {
	return counter_.fetch_add(1, std::memory_order_relaxed) + 1;
}

inline i32 Net::RefCounter::operator++(i32) {
	return counter_.fetch_add(1, std::memory_order_relaxed);
}

inline i32 Net::RefCounter::operator--() {
	return counter_.fetch_sub(1, std::memory_order_acquire) - 1;
}

inline i32 Net::RefCounter::operator--(i32) {
	return counter_.fetch_sub(1, std::memory_order_acquire);
}

inline Net::RefCounter::operator int() {
	return counter_;
}

//*********************************************************************
//RefCountedObject
//*********************************************************************

inline void Net::RefCountedObject::Duplicate() const {
	counter_++;
}

inline void Net::RefCountedObject::Release() {
	if (counter_ == 0) {
		Log(kCrash, __FILE__, __LINE__, "counter_ == 0");
	}
	if (--counter_ == 0) {
		delete this;
	}
}

inline i32 Net::RefCountedObject::ReferenceCount() const {
	return counter_;
}

//*********************************************************************
//WeakReference
//*********************************************************************
inline StrongRefObject * WeakReference::Get() const {
	return object_;
}

//*********************************************************************
//StrongRefObject
//*********************************************************************
inline WeakReference * StrongRefObject::WeakRef() {
	if (!weak_reference_) {
		weak_reference_ = new WeakReference();
		weak_reference_->object_ = this;
	}
	weak_reference_->Duplicate();
	return weak_reference_;
}

inline void StrongRefObject::ClearWeakReferences() {
	if (weak_reference_) {
		weak_reference_->object_ = nullptr;
		weak_reference_->Release();
		weak_reference_ = nullptr;
	}
}

}

#endif
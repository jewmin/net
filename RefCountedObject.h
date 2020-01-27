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

#ifndef Net_RefCountedObject_INCLUDED
#define Net_RefCountedObject_INCLUDED

#include "Logger.h"
#include "NetObject.h"

namespace Net {

class RefCounter {
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

class RefCountedObject : public NetObject {
public:
	RefCountedObject() {}
	virtual ~RefCountedObject() {}

	void Duplicate() const;
	void Release();
	i32 ReferenceCount() const;

private:
	RefCountedObject(RefCountedObject &&) = delete;
	RefCountedObject(const RefCountedObject &) = delete;
	RefCountedObject & operator=(RefCountedObject &&) = delete;
	RefCountedObject & operator=(const RefCountedObject &) = delete;

private:
	mutable RefCounter counter_;
};

template<class C>
class RefPtr {
public:
	RefPtr() : ptr_(nullptr) {}

	explicit RefPtr(C * ptr) : ptr_(ptr) {}

	RefPtr(const RefPtr & rhs) : ptr_(rhs.ptr_) {
		if (ptr_) {
			ptr_->Duplicate();
		}
	}

	RefPtr & operator=(C * ptr) {
		if (ptr_ != ptr) {
			if (ptr_) {
				ptr_->Release();
			}
			ptr_ = ptr;
		}
		return *this;
	}

	RefPtr & operator=(const RefPtr & ptr) {
		if (this != &ptr) {
			if (ptr_) {
				ptr_->Release();
			}
			ptr_ = ptr.ptr_;
			if (ptr_) {
				ptr_->Duplicate();
			}
		}
		return *this;
	}

	~RefPtr() {
		if (ptr_) {
			ptr_->Release();
		}
	}

	void Swap(RefPtr & ptr);
	C * Duplicate();

	C * operator->();
	const C * operator->() const;
	C & operator*();
	const C & operator*() const;
	C * Get();
	const C * Get() const;
	operator C * ();
	operator const C * () const;
	bool operator!() const;
	bool operator==(C * ptr) const;
	bool operator==(const C * ptr) const;
	bool operator==(const RefPtr & ptr) const;
	bool operator!=(C * ptr) const;
	bool operator!=(const C * ptr) const;
	bool operator!=(const RefPtr & ptr) const;
	bool operator<(C * ptr) const;
	bool operator<(const C * ptr) const;
	bool operator<(const RefPtr & ptr) const;
	bool operator<=(C * ptr) const;
	bool operator<=(const C * ptr) const;
	bool operator<=(const RefPtr & ptr) const;
	bool operator>(C * ptr) const;
	bool operator>(const C * ptr) const;
	bool operator>(const RefPtr & ptr) const;
	bool operator>=(C * ptr) const;
	bool operator>=(const C * ptr) const;
	bool operator>=(const RefPtr & ptr) const;

protected:
	C * Deref() const;

private:
	C * ptr_;
};

template<class T>
using AutoPtr = RefPtr<T>;

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
	if (--counter_ == 0) {
		delete this;
	}
}

inline i32 Net::RefCountedObject::ReferenceCount() const {
	return counter_;
}

//*********************************************************************
//RefPtr
//*********************************************************************

template<class C>
inline void Net::RefPtr<C>::Swap(RefPtr & ptr) {
	std::swap(ptr_, ptr.ptr_);
}

template<class C>
inline C * Net::RefPtr<C>::Duplicate() {
	if (ptr_) {
		ptr_->Duplicate();
	}
	return ptr_;
}

template<class C>
inline C * Net::RefPtr<C>::Deref() const {
	if (!ptr_) {
		Log(kCrash, __FILE__, __LINE__, "ptr_ is null");
	}
	return ptr_;
}

template<class C>
inline C * Net::RefPtr<C>::operator->() {
	return Deref();
}

template<class C>
inline const C * Net::RefPtr<C>::operator->() const {
	return Deref();
}

template<class C>
inline C & Net::RefPtr<C>::operator*() {
	return *Deref();
}

template<class C>
inline const C & Net::RefPtr<C>::operator*() const {
	return *Deref();
}

template<class C>
inline C * Net::RefPtr<C>::Get() {
	return ptr_;
}

template<class C>
inline const C * Net::RefPtr<C>::Get() const {
	return ptr_;
}

template<class C>
inline Net::RefPtr<C>::operator C*() {
	return ptr_;
}

template<class C>
inline Net::RefPtr<C>::operator const C*() const {
	return ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator!() const {
	return nullptr == ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator==(C * ptr) const {
	return ptr_ == ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator==(const C * ptr) const {
	return ptr_ == ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator==(const RefPtr & ptr) const {
	return ptr_ == ptr.ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator!=(C * ptr) const {
	return ptr_ != ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator!=(const C * ptr) const {
	return ptr_ != ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator!=(const RefPtr & ptr) const {
	return ptr_ != ptr.ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator<(C * ptr) const {
	return ptr_ < ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator<(const C * ptr) const {
	return ptr_ < ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator<(const RefPtr & ptr) const {
	return ptr_ < ptr.ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator<=(C * ptr) const {
	return ptr_ <= ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator<=(const C * ptr) const {
	return ptr_ <= ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator<=(const RefPtr & ptr) const {
	return ptr_ <= ptr.ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator>(C * ptr) const {
	return ptr_ > ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator>(const C * ptr) const {
	return ptr_ > ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator>(const RefPtr & ptr) const {
	return ptr_ > ptr.ptr_;
}

template<class C>
inline bool Net::RefPtr<C>::operator>=(C * ptr) const {
	return ptr_ >= ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator>=(const C * ptr) const {
	return ptr_ >= ptr;
}

template<class C>
inline bool Net::RefPtr<C>::operator>=(const RefPtr & ptr) const {
	return ptr_ >= ptr.ptr_;
}

}

#endif
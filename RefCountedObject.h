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

	template<class C>
	class RefPtr {
	public:
		RefPtr() : ptr_(nullptr) {
		}

		RefPtr(C * ptr) : ptr_(ptr) {
		}

		RefPtr(C * ptr, bool shared) : ptr_(ptr) {
			if (shared && ptr_) {
				ptr_->Duplicate();
			}
		}

		RefPtr(const RefPtr & ptr) : ptr_(ptr.ptr_) {
			if (ptr_) {
				ptr_->Duplicate();
			}
		}

		RefPtr(RefPtr && ptr) : ptr_(ptr.ptr_) {
			ptr.ptr_ = nullptr;
		}

		RefPtr & operator=(C * ptr) {
			return Assign(ptr);
		}

		RefPtr & operator=(const RefPtr & ptr) {
			return Assign(ptr);
		}

		RefPtr & operator=(RefPtr && ptr) {
			MoveAssign(&ptr.ptr_);
			return *this;
		}

		~RefPtr() {
			Release();
		}

		RefPtr & Assign(C * ptr);
		RefPtr & Assign(C * ptr, bool shared);
		RefPtr & Assign(const RefPtr & ptr);
		void MoveAssign(C * * ptr);
		void Reset();
		void Reset(C * ptr);
		void Reset(C * ptr, bool shared);
		void Reset(const RefPtr & ptr);
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
		void Release();

	private:
		C * ptr_;
	};

	template<class T>
	using AutoPtr = RefPtr<T>;
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

//*********************************************************************
//RefPtr
//*********************************************************************

template<class C>
inline Foundation::RefPtr<C> & Foundation::RefPtr<C>::Assign(C * ptr) {
	if (ptr_ != ptr) {
		Release();
		ptr_ = ptr;
	}

	return *this;
}

template<class C>
inline Foundation::RefPtr<C> & Foundation::RefPtr<C>::Assign(C * ptr, bool shared) {
	if (ptr_ != ptr) {
		Release();
		ptr_ = ptr;
		if (shared && ptr_) {
			ptr_->Duplicate();
		}
	}

	return *this;
}

template<class C>
inline Foundation::RefPtr<C> & Foundation::RefPtr<C>::Assign(const RefPtr & ptr) {
	if (this != &ptr) {
		Release();
		ptr_ = ptr.ptr_;
		if (ptr_) {
			ptr_->Duplicate();
		}
	}

	return *this;
}

template<class C>
inline void Foundation::RefPtr<C>::MoveAssign(C ** ptr) {
	if (ptr_ != *ptr) {
		Reset();
		if (*ptr) {
			ptr_ = *ptr;
			*ptr = nullptr;
		}
	}
}

template<class C>
inline void Foundation::RefPtr<C>::Reset() {
	if (ptr_) {
		Release();
		ptr_ = nullptr;
	}
}

template<class C>
inline void Foundation::RefPtr<C>::Reset(C * ptr) {
	Assign(ptr);
}

template<class C>
inline void Foundation::RefPtr<C>::Reset(C * ptr, bool shared) {
	Assign(ptr, shared);
}

template<class C>
inline void Foundation::RefPtr<C>::Reset(const RefPtr & ptr) {
	Assign(ptr);
}

template<class C>
inline void Foundation::RefPtr<C>::Swap(RefPtr & ptr) {
	std::swap(ptr_, ptr.ptr_);
}

template<class C>
inline C * Foundation::RefPtr<C>::Duplicate() {
	if (ptr_) {
		ptr_->Duplicate();
	}

	return ptr_;
}

template<class C>
inline C * Foundation::RefPtr<C>::Deref() const {
	if (ptr_) {
		return ptr_;
	}
	else {
		throw std::invalid_argument("Deref(): ptr_ is null");
	}
}

template<class C>
inline void Foundation::RefPtr<C>::Release() {
	if (ptr_) {
		ptr_->Release();
	}
}

template<class C>
inline C * Foundation::RefPtr<C>::operator->() {
	return Deref();
}

template<class C>
inline const C * Foundation::RefPtr<C>::operator->() const {
	return Deref();
}

template<class C>
inline C & Foundation::RefPtr<C>::operator*() {
	return *Deref();
}

template<class C>
inline const C & Foundation::RefPtr<C>::operator*() const {
	return *Deref();
}

template<class C>
inline C * Foundation::RefPtr<C>::Get() {
	return ptr_;
}

template<class C>
inline const C * Foundation::RefPtr<C>::Get() const {
	return ptr_;
}

template<class C>
inline Foundation::RefPtr<C>::operator C*() {
	return ptr_;
}

template<class C>
inline Foundation::RefPtr<C>::operator const C*() const {
	return ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator!() const {
	return nullptr == ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator==(C * ptr) const {
	return ptr_ == ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator==(const C * ptr) const {
	return ptr_ == ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator==(const RefPtr & ptr) const {
	return ptr_ == ptr.ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator!=(C * ptr) const {
	return ptr_ != ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator!=(const C * ptr) const {
	return ptr_ != ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator!=(const RefPtr & ptr) const {
	return ptr_ != ptr.ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<(C * ptr) const {
	return ptr_ < ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<(const C * ptr) const {
	return ptr_ < ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<(const RefPtr & ptr) const {
	return ptr_ < ptr.ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<=(C * ptr) const {
	return ptr_ <= ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<=(const C * ptr) const {
	return ptr_ <= ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator<=(const RefPtr & ptr) const {
	return ptr_ <= ptr.ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>(C * ptr) const {
	return ptr_ > ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>(const C * ptr) const {
	return ptr_ > ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>(const RefPtr & ptr) const {
	return ptr_ > ptr.ptr_;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>=(C * ptr) const {
	return ptr_ >= ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>=(const C * ptr) const {
	return ptr_ >= ptr;
}

template<class C>
inline bool Foundation::RefPtr<C>::operator>=(const RefPtr & ptr) const {
	return ptr_ >= ptr.ptr_;
}

#endif
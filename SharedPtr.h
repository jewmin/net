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

#ifndef Foundation_SharedPtr_INCLUDED
#define Foundation_SharedPtr_INCLUDED

#include "CommonDef.h"

namespace Foundation {
	template<class C>
	class SharedPtr {
	public:
		SharedPtr() {
		}

		SharedPtr(C * ptr) : ptr_(ptr) {
		}

		SharedPtr(const std::shared_ptr<C> & ptr) : ptr_(ptr) {
		}

		SharedPtr(const SharedPtr & ptr) : ptr_(ptr.ptr_) {
		}

		SharedPtr(SharedPtr && ptr) : ptr_(std::move(ptr.ptr_)) {
		}

		SharedPtr(std::shared_ptr<C> && ptr) : ptr_(std::move(ptr)) {
		}

		SharedPtr & operator=(C * ptr) {
			return Assign(ptr);
		}

		SharedPtr & operator=(const SharedPtr & ptr) {
			return Assign(ptr);
		}

		SharedPtr & operator=(SharedPtr && ptr) {
			ptr_ = std::move(ptr.ptr_);
			return *this;
		}

		SharedPtr & operator=(std::shared_ptr<C> && ptr) {
			ptr_ = std::move(ptr);
			return *this;
		}

		~SharedPtr() {
		}

		SharedPtr & Assign(C * ptr);
		SharedPtr & Assign(const SharedPtr & ptr);
		void Reset();
		void Reset(C * ptr);
		void Reset(const SharedPtr & ptr);
		void Swap(SharedPtr & ptr);
		long ReferenceCount() const;
		
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
		bool operator==(const SharedPtr & ptr) const;
		bool operator!=(C * ptr) const;
		bool operator!=(const C * ptr) const;
		bool operator!=(const SharedPtr & ptr) const;
		bool operator<(C * ptr) const;
		bool operator<(const C * ptr) const;
		bool operator<(const SharedPtr & ptr) const;
		bool operator<=(C * ptr) const;
		bool operator<=(const C * ptr) const;
		bool operator<=(const SharedPtr & ptr) const;
		bool operator>(C * ptr) const;
		bool operator>(const C * ptr) const;
		bool operator>(const SharedPtr & ptr) const;
		bool operator>=(C * ptr) const;
		bool operator>=(const C * ptr) const;
		bool operator>=(const SharedPtr & ptr) const;

	protected:
		C * Deref() const;

	private:
		std::shared_ptr<C> ptr_;
	};
}

template<class C>
inline Foundation::SharedPtr<C> & Foundation::SharedPtr<C>::Assign(C * ptr) {
	ptr_.reset(ptr);
	return *this;
}

template<class C>
inline Foundation::SharedPtr<C> & Foundation::SharedPtr<C>::Assign(const SharedPtr & ptr) {
	if (this != &ptr) {
		SharedPtr tmp(ptr);
		Swap(tmp);
	}

	return *this;
}

template<class C>
inline void Foundation::SharedPtr<C>::Reset() {
	ptr_.reset();
}

template<class C>
inline void Foundation::SharedPtr<C>::Reset(C * ptr) {
	ptr_.reset(ptr);
}

template<class C>
inline void Foundation::SharedPtr<C>::Reset(const SharedPtr & ptr) {
	Assign(ptr);
}

template<class C>
inline void Foundation::SharedPtr<C>::Swap(SharedPtr & ptr) {
	std::swap(ptr_, ptr.ptr_);
}

template<class C>
inline long Foundation::SharedPtr<C>::ReferenceCount() const {
	return ptr_.use_count();
}

template<class C>
inline C * Foundation::SharedPtr<C>::Deref() const {
	if (ptr_) {
		return ptr_.get();
	} else {
		throw std::invalid_argument("Deref(): ptr_ is null");
	}
}

template<class C>
inline C * Foundation::SharedPtr<C>::operator->() {
	return Deref();
}

template<class C>
inline const C * Foundation::SharedPtr<C>::operator->() const {
	return Deref();
}

template<class C>
inline C & Foundation::SharedPtr<C>::operator*() {
	return *Deref();
}

template<class C>
inline const C & Foundation::SharedPtr<C>::operator*() const {
	return *Deref();
}

template<class C>
inline C * Foundation::SharedPtr<C>::Get() {
	return ptr_.get();
}

template<class C>
inline const C * Foundation::SharedPtr<C>::Get() const {
	return ptr_.get();
}

template<class C>
inline Foundation::SharedPtr<C>::operator C*() {
	return ptr_.get();
}

template<class C>
inline Foundation::SharedPtr<C>::operator const C*() const {
	return ptr_.get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator!() const {
	return nullptr == ptr_;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator==(C * ptr) const {
	return Get() == ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator==(const C * ptr) const {
	return Get() == ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator==(const SharedPtr & ptr) const {
	return Get() == ptr.Get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator!=(C * ptr) const {
	return Get() != ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator!=(const C * ptr) const {
	return Get() != ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator!=(const SharedPtr & ptr) const {
	return Get() != ptr.Get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<(C * ptr) const {
	return Get() < ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<(const C * ptr) const {
	return Get() < ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<(const SharedPtr & ptr) const {
	return Get() < ptr.Get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<=(C * ptr) const {
	return Get() <= ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<=(const C * ptr) const {
	return Get() <= ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator<=(const SharedPtr & ptr) const {
	return Get() <= ptr.Get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>(C * ptr) const {
	return Get() > ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>(const C * ptr) const {
	return Get() > ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>(const SharedPtr & ptr) const {
	return Get() > ptr.Get();
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>=(C * ptr) const {
	return Get() >= ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>=(const C * ptr) const {
	return Get() >= ptr;
}

template<class C>
inline bool Foundation::SharedPtr<C>::operator>=(const SharedPtr & ptr) const {
	return Get() >= ptr.Get();
}

#endif
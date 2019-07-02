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

#include "Observer.h"

template<class C, class N>
Foundation::Observer<C, N>::Observer(C & object, Callback method)
	: object_(object), method_(method) {
}

template<class C, class N>
Foundation::Observer<C, N>::Observer(const Observer & rhs)
	: AbstractObserver(rhs), object_(rhs.object_), method_(rhs.method_) {
}

template<class C, class N>
Foundation::Observer<C, N> & Foundation::Observer<C, N>::operator=(const Observer & rhs) {
	if (this != &rhs) {
		object_ = rhs.object_;
		method_ = rhs.method_;
	}

	return *this;
}

template<class C, class N>
Foundation::Observer<C, N>::~Observer() {
}

template<class C, class N>
void Foundation::Observer<C, N>::Notify(Notification * notification) const {
	if (object_) {
		N * cast_notification = dynamic_cast<N *>(notification);
		if (cast_notification) {
			cast_notification->Duplicate();
			(object_->*method_)(cast_notification);
		}
	}
}

template<class C, class N>
bool Foundation::Observer<C, N>::Equals(const AbstractObserver & observer) const {
	const Observer * obs = dynamic_cast<const Observer *>(&observer);
	return obs && obs->object_ == object_ && obs->method_ == method_;
}

template<class C, class N>
bool Foundation::Observer<C, N>::Accepts(Notification * notification) const {
	return dynamic_cast<N *>(notification) != nullptr;
}

template<class C, class N>
Foundation::AbstractObserver * Foundation::Observer<C, N>::Clone() const {
	return new Observer(*this);
}

template<class C, class N>
void Foundation::Observer<C, N>::Disable() {
	object_ = nullptr;
}
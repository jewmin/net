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

#ifndef Foundation_Observer_INCLUDED
#define Foundation_Observer_INCLUDED

#include "AbstractObserver.h"

namespace Foundation {
	template<class C, class N>
	class Observer : public AbstractObserver {
	public:
		typedef void (C::*Callback)(N *);

		Observer() = delete;
		Observer(C & object, Callback method);
		Observer(const Observer & rhs);
		Observer & operator=(const Observer & rhs);
		~Observer();

		virtual void Notify(Notification * notification) const;
		virtual bool Equals(const AbstractObserver & observer) const;
		virtual bool Accepts(Notification * notification) const;
		virtual AbstractObserver * Clone() const;
		virtual void Disable();

	private:
		C * object_;
		Callback method_;
	};
}

#endif
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

#ifndef Foundation_AbstractObserver_INCLUDED
#define Foundation_AbstractObserver_INCLUDED

#include "Notification.h"

namespace Foundation {
	class AbstractObserver {
	public:
		AbstractObserver();
		AbstractObserver(const AbstractObserver & rhs);
		AbstractObserver & operator=(const AbstractObserver & rhs);
		~AbstractObserver();

		virtual void Notify(Notification * notification) const = 0;
		virtual bool Equals(const AbstractObserver & observer) const = 0;
		virtual bool Accepts(Notification * notification) const = 0;
		virtual AbstractObserver * Clone() const = 0;
		virtual void Disable() = 0;
	};
}

#endif
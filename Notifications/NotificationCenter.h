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

#ifndef Foundation_NotificationCenter_INCLUDED
#define Foundation_NotificationCenter_INCLUDED

#include "AbstractObserver.h"
#include "SharedPtr.h"

namespace Foundation {
	class NotificationCenter {
	public:
		NotificationCenter();
		~NotificationCenter();

		void AddObserver(const AbstractObserver & observer);
		void RemoveObserver(const AbstractObserver & observer);
		bool HasObserver(const AbstractObserver & observer) const;
		bool HasObservers() const;
		std::size_t CountObservers() const;
		void PostNotification(Notification::Ptr notification);

	private:
		typedef Foundation::SharedPtr<AbstractObserver> AbstractObserverPtr;
		typedef std::vector<AbstractObserverPtr> ObserverList;

		ObserverList observers_;
	};
}

#endif
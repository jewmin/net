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

#include "NotificationCenter.h"

Foundation::NotificationCenter::NotificationCenter() {
}

Foundation::NotificationCenter::~NotificationCenter() {
}

void Foundation::NotificationCenter::AddObserver(const AbstractObserver & observer) {
	observers_.push_back(observer.Clone());
}

void Foundation::NotificationCenter::RemoveObserver(const AbstractObserver & observer) {
	for (ObserverList::iterator it = observers_.begin(); it != observers_.end(); ++it) {
		if (observer.Equals(**it)) {
			(*it)->Disable();
			observers_.erase(it);
			return;
		}
	}
}

bool Foundation::NotificationCenter::HasObserver(const AbstractObserver & observer) const {
	for (ObserverList::const_iterator it = observers_.begin(); it != observers_.end(); ++it) {
		if (observer.Equals(**it)) {
			return true;
		}
	}

	return false;
}

bool Foundation::NotificationCenter::HasObservers() const {
	return !observers_.empty();
}

std::size_t Foundation::NotificationCenter::CountObservers() const {
	return observers_.size();
}

void Foundation::NotificationCenter::PostNotification(Notification::Ptr notification) {
	for (ObserverList::const_iterator it = observers_.begin(); it != observers_.end(); ++it) {
		(*it)->Notify(notification);
	}
}
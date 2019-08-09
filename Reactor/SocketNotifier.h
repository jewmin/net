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

#ifndef Net_SocketNotifier_INCLUDED
#define Net_SocketNotifier_INCLUDED

#include "RefCountedObject.h"
#include "Sockets/Socket.h"
#include "Notifications/AbstractObserver.h"
#include "SocketNotification.h"
#include "Notifications/NotificationCenter.h"

namespace Net {
	class SocketReactor;
	class SocketNotifier : public Foundation::RefCountedObject {
	public:
		explicit SocketNotifier(const Socket & socket);

		void AddObserver(SocketReactor * reactor, const Foundation::AbstractObserver & observer);
		void RemoveObserver(SocketReactor * reactor, const Foundation::AbstractObserver & observer);
		bool HasObserver(const Foundation::AbstractObserver & observer) const;
		bool HasObservers() const;
		void Dispatch(SocketNotification * notification);
		std::size_t CountObservers() const;

	protected:
		~SocketNotifier();

	private:
		Foundation::NotificationCenter nc_;
		Socket socket_;
	};
}

inline bool Net::SocketNotifier::HasObserver(const Foundation::AbstractObserver & observer) const {
	return nc_.HasObserver(observer);
}

inline bool Net::SocketNotifier::HasObservers() const {
	return nc_.HasObservers();
}

inline std::size_t Net::SocketNotifier::CountObservers() const {
	return nc_.CountObservers();
}

#endif
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

#ifndef Net_SocketReactor_INCLUDED
#define Net_SocketReactor_INCLUDED

#include "Sockets/Socket.h"
#include "Notifications/AbstractObserver.h"
#include "SocketNotification.h"
#include "SocketNotifier.h"

namespace Net {
	class SocketReactor {
	public:
		SocketReactor();
		~SocketReactor();

		void Run();
		void Stop();
		void AddEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer);
		bool HasEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer);
		void RemoveEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer);
		void Dispatch(SocketNotification * notification);
		void Dispatch(const Socket & socket, SocketNotification * notification);
		SocketNotification * GetNotification(SocketNotification::NotificationType t);
		uv_loop_t * GetLoop();

	private:
		typedef Foundation::AutoPtr<SocketNotifier> NotifierPtr;
		typedef Foundation::AutoPtr<SocketNotification> NotificationPtr;
		typedef std::map<Socket, NotifierPtr> EventHandlerMap;

		void Dispatch(NotifierPtr & notifier, SocketNotification * notification);
		NotifierPtr GetNotifier(const Socket & socket, bool makeNew = false);

		uv_loop_t loop_;
		EventHandlerMap handlers_;
		NotificationPtr readable_notification_;
		NotificationPtr writable_notification_;
		NotificationPtr error_notification_;
		NotificationPtr timeout_notification_;
		NotificationPtr shutdown_notification_;
	};
}

inline uv_loop_t * Net::SocketReactor::GetLoop() {
	return &loop_;
}

#endif
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

#ifndef Net_SocketNotification_INCLUDED
#define Net_SocketNotification_INCLUDED

#include "Notifications/Notification.h"

namespace Net {
	class SocketNotification : public Foundation::Notification {
	public:
		enum NotificationType { kNone, kRead, kWrite, kError, kTimeout, kShutdown };
		SocketNotification();
		SocketNotification(NotificationType t);
		virtual ~SocketNotification();
		NotificationType GetType();
		
	protected:
		NotificationType type_;
	};

	class ReadableNotification : public SocketNotification {
	public:
		ReadableNotification();
		~ReadableNotification();
	};

	class WritableNotification : public SocketNotification {
	public:
		WritableNotification();
		~WritableNotification();
	};

	class ErrorNotification : public SocketNotification {
	public:
		ErrorNotification();
		~ErrorNotification();
	};

	class TimeoutNotification : public SocketNotification {
	public:
		TimeoutNotification();
		~TimeoutNotification();
	};

	class ShutdownNotification : public SocketNotification {
	public:
		ShutdownNotification();
		~ShutdownNotification();
	};
}

inline Net::SocketNotification::NotificationType Net::SocketNotification::GetType() {
	return type_;
}

#endif
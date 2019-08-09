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

#include "SocketReactor.h"

Net::SocketReactor::SocketReactor()
	: readable_notification_(new ReadableNotification())
	, writable_notification_(new WritableNotification())
	, error_notification_(new ErrorNotification())
	, timeout_notification_(new TimeoutNotification())
	, shutdown_notification_(new ShutdownNotification()) {
	int err = uv_loop_init(&loop_);
	if (0 != err) {
		std::stringstream ss;
		ss << "uv_loop_init error: " << uv_strerror(err);
		throw std::logic_error(ss.str());
	}
	loop_.data = this;
}

Net::SocketReactor::~SocketReactor() {
	while (uv_run(&loop_, UV_RUN_NOWAIT)) {
		uv_run(&loop_, UV_RUN_ONCE);
	}

	uv_loop_close(&loop_);
}

void Net::SocketReactor::Run() {
	uv_run(&loop_, UV_RUN_NOWAIT);
}

void Net::SocketReactor::Stop() {
	uv_stop(&loop_);
	Dispatch(shutdown_notification_);
}

void Net::SocketReactor::AddEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer) {
	NotifierPtr notifier = GetNotifier(socket, true);
	if (!notifier->HasObserver(observer)) {
		notifier->AddObserver(this, observer);
	}
}

bool Net::SocketReactor::HasEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer) {
	NotifierPtr notifier = GetNotifier(socket);
	if (notifier && notifier->HasObserver(observer)) {
		return true;
	}

	return false;
}

void Net::SocketReactor::RemoveEventHandler(const Socket & socket, const Foundation::AbstractObserver & observer) {
	NotifierPtr notifier = GetNotifier(socket);
	if (notifier && notifier->HasObserver(observer)) {
		if (notifier->CountObservers() == 1) {
			handlers_.erase(socket);
		}
		notifier->RemoveObserver(this, observer);
	}
}

void Net::SocketReactor::Dispatch(SocketNotification * notification) {
	std::vector<NotifierPtr> delegates;
	delegates.reserve(handlers_.size());
	for (EventHandlerMap::iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
		delegates.push_back(it->second);
	}

	for (std::vector<NotifierPtr>::iterator it = delegates.begin(); it != delegates.end(); ++it) {
		Dispatch(*it, notification);
	}
}

void Net::SocketReactor::Dispatch(const Socket & socket, SocketNotification * notification) {
	NotifierPtr notifier = GetNotifier(socket);
	if (notifier) {
		Dispatch(notifier, notification);
	}
}

Net::SocketNotification * Net::SocketReactor::GetNotification(SocketNotification::NotificationType t) {
	switch (t) {
	case Net::SocketNotification::kRead:
		return readable_notification_;
	
	case Net::SocketNotification::kWrite:
		return writable_notification_;

	case Net::SocketNotification::kError:
		return error_notification_;

	case Net::SocketNotification::kTimeout:
		return timeout_notification_;

	case Net::SocketNotification::kShutdown:
		return shutdown_notification_;

	default:
		return nullptr;
	}
}

void Net::SocketReactor::Dispatch(NotifierPtr & notifier, SocketNotification * notification) {
	notifier->Dispatch(notification);
}

Net::SocketReactor::NotifierPtr Net::SocketReactor::GetNotifier(const Socket & socket, bool makeNew) {
	EventHandlerMap::iterator it = handlers_.find(socket);
	if (it != handlers_.end()) {
		return it->second;
	} else if (makeNew) {
		return (handlers_[socket] = new SocketNotifier(socket));
	} else {
		return nullptr;
	}
}
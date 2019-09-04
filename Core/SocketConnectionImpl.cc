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

#include "SocketConnectionImpl.h"

Net::SocketConnectionImpl::SocketConnectionImpl(INotification * notification, int maxOutBufferSize, int maxInBufferSize)
	: SocketConnection(maxOutBufferSize, maxInBufferSize), notification_(notification) {
	if (!notification_) {
		throw std::invalid_argument("notification is nullptr!");
	}
}

Net::SocketConnectionImpl::~SocketConnectionImpl() {
}

void Net::SocketConnectionImpl::Destroy() {
	notification_ = nullptr;
	SocketConnection::Destroy();
}

void Net::SocketConnectionImpl::OnConnected() {
	if (notification_) {
		notification_->OnConnected();
	}
}

void Net::SocketConnectionImpl::OnConnectFailed(int reason) {
	if (notification_) {
		notification_->OnConnectFailed(reason);
	}
}

void Net::SocketConnectionImpl::OnDisconnected(bool isRemote) {
	if (notification_) {
		notification_->OnDisconnected(isRemote);
	}
}

void Net::SocketConnectionImpl::OnNewDataReceived() {
	if (notification_) {
		notification_->OnNewDataReceived();
	}
}

void Net::SocketConnectionImpl::OnSomeDataSent() {
	if (notification_) {
		notification_->OnSomeDataSent();
	}
}

void Net::SocketConnectionImpl::OnError(int reason) {
	if (notification_) {
		notification_->OnError(reason);
	}
}
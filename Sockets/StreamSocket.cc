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

#include "StreamSocket.h"
#include "StreamSocketImpl.h"

Net::StreamSocket::StreamSocket(uv_loop_t * loop)
	: Socket(new StreamSocketImpl(loop)) {
}

Net::StreamSocket::StreamSocket(SocketImpl * impl)
	: Socket(impl) {
	if (!dynamic_cast<StreamSocketImpl *>(Impl())) {
		throw std::invalid_argument("Cannot assign incompatible socket");
	}
}

Net::StreamSocket::StreamSocket(const Socket & socket)
	: Socket(socket) {
	if (!dynamic_cast<StreamSocketImpl *>(Impl())) {
		throw std::invalid_argument("Cannot assign incompatible socket");
	}
}

Net::StreamSocket & Net::StreamSocket::operator=(const Socket & socket) {
	if (dynamic_cast<StreamSocketImpl *>(socket.Impl())) {
		Socket::operator=(socket);
	} else {
		throw std::invalid_argument("Cannot assign incompatible socket");
	}

	return *this;
}

Net::StreamSocket::~StreamSocket() {
}

void Net::StreamSocket::Connect(const SocketAddress & address, const int timeout) {
	Impl()->Connect(address, timeout);
}

void Net::StreamSocket::Bind(const SocketAddress & address, bool ipV6Only) {
	Impl()->Bind(address, true, ipV6Only);
}

void Net::StreamSocket::ShutdownReceive() {
	Impl()->ShutdownReceive();
}

void Net::StreamSocket::ShutdownSend() {
	Impl()->ShutdownSend();
}

void Net::StreamSocket::Shutdown() {
	Impl()->Shutdown();
}

int Net::StreamSocket::SendBytes(const void * buffer, int length) {
	return Impl()->SendBytes(buffer, length);
}

int Net::StreamSocket::ReceiveBytes(void * buffer, int length) {
	return Impl()->ReceiveBytes(buffer, length);
}

Net::SocketAddress Net::StreamSocket::PeerAddress() const {
	return Impl()->PeerAddress();
}

void Net::StreamSocket::SetNoDelay(bool flag) {
	Impl()->SetNoDelay(flag);
}

void Net::StreamSocket::SetKeepAlive(bool flag, int interval) {
	Impl()->SetKeepAlive(flag, interval);
}
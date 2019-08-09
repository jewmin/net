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

#include "ServerSocket.h"
#include "ServerSocketImpl.h"

Net::ServerSocket::ServerSocket(uv_loop_t * loop)
	: Socket(new ServerSocketImpl(loop)) {
}

Net::ServerSocket::ServerSocket(const Socket & socket)
	: Socket(socket) {
	if (!dynamic_cast<ServerSocketImpl *>(Impl())) {
		throw std::invalid_argument("Cannot assign incompatible socket");
	}
}

Net::ServerSocket & Net::ServerSocket::operator=(const Socket & socket) {
	if (dynamic_cast<ServerSocketImpl *>(socket.Impl())) {
		Socket::operator=(socket);
	} else {
		throw std::invalid_argument("Cannot assign incompatible socket");
	}

	return *this;
}

Net::ServerSocket::~ServerSocket() {
}

void Net::ServerSocket::Bind(const SocketAddress & address, bool ipV6Only) {
	Impl()->Bind(address, true, ipV6Only);
}

void Net::ServerSocket::Listen(int backlog) {
	Impl()->Listen(backlog);
}

Net::StreamSocket Net::ServerSocket::AcceptConnection(SocketAddress & clientAddr) {
	return StreamSocket(Impl()->AcceptConnection(clientAddr));
}

Net::StreamSocket Net::ServerSocket::AcceptConnection() {
	SocketAddress client_addr;
	return StreamSocket(Impl()->AcceptConnection(client_addr));
}
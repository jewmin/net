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

#include "Sockets/ServerSocket.h"
#include "Sockets/ServerSocketImpl.h"

namespace Net {

ServerSocket::ServerSocket() : Socket(new ServerSocketImpl()) {
}

ServerSocket::ServerSocket(const Socket & rhs) : Socket(rhs) {
	if (!dynamic_cast<ServerSocketImpl *>(Impl())) {
		Log(kCrash, __FILE__, __LINE__, "socket impl != ServerSocketImpl");
	}
}

ServerSocket & ServerSocket::operator=(const Socket & rhs) {
	if (dynamic_cast<ServerSocketImpl *>(rhs.Impl())) {
		Socket::operator=(rhs);
	} else {
		Log(kCrash, __FILE__, __LINE__, "socket impl != ServerSocketImpl");
	}
	return *this;
}

ServerSocket::~ServerSocket() {
}

bool ServerSocket::AcceptSocket(StreamSocket & socket, SocketAddress & client_address) {
	SocketImpl * client = Impl()->AcceptSocket(client_address);
	if (client) {
		socket = StreamSocket(client);
		return true;
	}
	return false;
}

bool ServerSocket::AcceptSocket(StreamSocket & socket) {
	SocketAddress client_address;
	return AcceptSocket(socket, client_address);
}

}
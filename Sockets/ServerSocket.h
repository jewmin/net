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

#ifndef Net_ServerSocket_INCLUDED
#define Net_ServerSocket_INCLUDED

#include "Socket.h"
#include "StreamSocket.h"

namespace Net {
	class ServerSocket : public Socket {
	public:
		ServerSocket();
		ServerSocket(const Socket & socket);
		ServerSocket & operator=(const Socket & socket);
		virtual ~ServerSocket();

		int Bind(const SocketAddress & address, bool ipV6Only = false, bool reuseAddress = false);
		int Listen(int backlog, uv_connection_cb cb);
		bool AcceptConnection(StreamSocket & socket, SocketAddress & clientAddr);
		bool AcceptConnection(StreamSocket & socket);

	protected:
		explicit ServerSocket(SocketImpl * impl);
	};
}

inline int Net::ServerSocket::Bind(const SocketAddress & address, bool ipV6Only, bool reuseAddress) {
	return Impl()->Bind(address, ipV6Only, reuseAddress);
}

inline int Net::ServerSocket::Listen(int backlog, uv_connection_cb cb) {
	return Impl()->Listen(backlog, cb);
}

#endif
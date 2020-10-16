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

#ifndef Net_Sockets_ServerSocket_INCLUDED
#define Net_Sockets_ServerSocket_INCLUDED

#include "Common.h"
#include "Sockets/Socket.h"
#include "Sockets/StreamSocket.h"

namespace Net {

class COMMON_EXTERN ServerSocket : public Socket {
public:
	ServerSocket();
	ServerSocket(const Socket & other);
	ServerSocket & operator=(const Socket & other);
	virtual ~ServerSocket();

	i32 Bind(const SocketAddress & address, bool ipv6_only = false, bool reuse_address = false);
	i32 Listen(i32 backlog = 128);
	bool AcceptSocket(StreamSocket & socket, SocketAddress & client_address);
	bool AcceptSocket(StreamSocket & socket);
};

inline i32 ServerSocket::Bind(const SocketAddress & address, bool ipv6_only, bool reuse_address) {
	return Impl()->Bind(address, ipv6_only, reuse_address);
}

inline i32 ServerSocket::Listen(i32 backlog) {
	return Impl()->Listen(backlog);
}

}

#endif
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

#ifndef Net_Sockets_StreamSocket_INCLUDED
#define Net_Sockets_StreamSocket_INCLUDED

#include "Sockets/Socket.h"

namespace Net {

class StreamSocket : public Socket {
	friend class ServerSocket;

public:
	StreamSocket();
	StreamSocket(const Socket & rhs);
	StreamSocket & operator=(const Socket & rhs);
	virtual ~StreamSocket();

	i32 Bind(const SocketAddress & address, bool ipv6_only = false, bool reuse_address = false);
	i32 Connect(const SocketAddress & address, void * arg = nullptr);
	i32 Shutdown(void * arg = nullptr);
	i32 ShutdownWrite(void * arg = nullptr);
	i32 ShutdownRead();
	i32 Established();
	i32 Write(const i8 * data, i32 len, void * arg = nullptr);

protected:
	explicit StreamSocket(SocketImpl * impl);
};

inline i32 StreamSocket::Bind(const SocketAddress & address, bool ipv6_only, bool reuse_address) {
	return Impl()->Bind(address, ipv6_only, reuse_address);
}

inline i32 StreamSocket::Connect(const SocketAddress & address, void * arg) {
	return Impl()->Connect(address, arg);
}

inline i32 StreamSocket::Shutdown(void * arg) {
	return Impl()->Shutdown(arg);
}

inline i32 StreamSocket::ShutdownWrite(void * arg) {
	return Impl()->ShutdownWrite(arg);
}

inline i32 StreamSocket::ShutdownRead() {
	return Impl()->ShutdownRead();
}

inline i32 StreamSocket::Established() {
	return Impl()->Established();
}

inline i32 StreamSocket::Write(const i8 * data, i32 len, void * arg) {
	return Impl()->Write(data, len, arg);
}

}

#endif
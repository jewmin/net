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
	i32 Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb);
	void ShutdownReceive();
	void ShutdownSend(uv_shutdown_t * req, uv_shutdown_cb cb);
	void Shutdown(uv_shutdown_t * req, uv_shutdown_cb cb);
	i32 Established(uv_alloc_cb alloc_cb, uv_read_cb read_cb);
	i32 Send(const i8 * data, i32 len, uv_write_t * req, uv_write_cb cb);

protected:
	explicit StreamSocket(SocketImpl * impl);
};

inline i32 StreamSocket::Bind(const SocketAddress & address, bool ipv6_only, bool reuse_address) {
	return Impl()->Bind(address, ipv6_only, reuse_address);
}

inline i32 StreamSocket::Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb) {
	return Impl()->Connect(address, req, cb);
}

inline void StreamSocket::ShutdownReceive() {
	Impl()->ShutdownReceive();
}

inline void StreamSocket::ShutdownSend(uv_shutdown_t * req, uv_shutdown_cb cb) {
	Impl()->ShutdownSend(req, cb);
}

inline void StreamSocket::Shutdown(uv_shutdown_t * req, uv_shutdown_cb cb) {
	Impl()->Shutdown(req, cb);
}

inline i32 StreamSocket::Established(uv_alloc_cb alloc_cb, uv_read_cb read_cb) {
	return Impl()->Established(alloc_cb, read_cb);
}

inline i32 StreamSocket::Send(const i8 * data, i32 len, uv_write_t * req, uv_write_cb cb) {
	return Impl()->Send(data, len, req, cb);
}

}

#endif
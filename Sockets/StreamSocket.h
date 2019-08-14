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

#ifndef Net_StreamSocket_INCLUDED
#define Net_StreamSocket_INCLUDED

#include "Socket.h"

namespace Net {
	class StreamSocket : public Socket {
	public:
		StreamSocket();
		StreamSocket(const Socket & socket);
		StreamSocket & operator=(const Socket & socket);
		virtual ~StreamSocket();

		int Bind(const SocketAddress & address, bool ipV6Only = false, bool reuseAddress = false);
		int Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb);
		void ShutdownReceive();
		void ShutdownSend(uv_shutdown_cb cb = reinterpret_cast<uv_shutdown_cb>(free));
		void Shutdown(uv_shutdown_cb cb = reinterpret_cast<uv_shutdown_cb>(free));
		int Established(uv_alloc_cb alloc_cb, uv_read_cb read_cb);

	protected:
		explicit StreamSocket(SocketImpl * impl);

	private:
		friend class ServerSocket;
	};
}

inline int Net::StreamSocket::Bind(const SocketAddress & address, bool ipV6Only, bool reuseAddress) {
	return Impl()->Bind(address, ipV6Only, reuseAddress);
}

inline int Net::StreamSocket::Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb) {
	return Impl()->Connect(address, req, cb);
}

inline void Net::StreamSocket::ShutdownReceive() {
	Impl()->ShutdownReceive();
}

inline void Net::StreamSocket::ShutdownSend(uv_shutdown_cb cb) {
	Impl()->ShutdownSend(cb);
}

inline void Net::StreamSocket::Shutdown(uv_shutdown_cb cb) {
	Impl()->Shutdown(cb);
}

inline int Net::StreamSocket::Established(uv_alloc_cb allocCb, uv_read_cb readCb) {
	return Impl()->Established(allocCb, readCb);
}

#endif
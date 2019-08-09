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
		explicit StreamSocket(uv_loop_t * loop);
		explicit StreamSocket(SocketImpl * impl);
		StreamSocket(const Socket & socket);
		StreamSocket & operator=(const Socket & socket);
		virtual ~StreamSocket();

		void Connect(const SocketAddress & address, const int timeout = 0);
		void Bind(const SocketAddress & address, bool ipV6Only = false);
		void ShutdownReceive();
		void ShutdownSend();
		void Shutdown();
		int SendBytes(const void * buffer, int length);
		int ReceiveBytes(void * buffer, int length);
		SocketAddress PeerAddress() const;
		void SetNoDelay(bool flag);
		void SetKeepAlive(bool flag, int interval);

	private:
		friend class ServerSocket;
	};
}

#endif
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

#ifndef Net_DatagramSocketImpl_INCLUDED
#define Net_DatagramSocketImpl_INCLUDED

#include "SocketImpl.h"

namespace Net {
	class DatagramSocketImpl : public SocketImpl {
	public:
		explicit DatagramSocketImpl(uv_loop_t * loop);

		virtual void Bind(const SocketAddress & address, bool reuseAddress = false, bool ipV6Only = false);
		virtual void ShutdownReceive();
		virtual void StartRead();
		virtual int SendTo(const void * buffer, int length, const SocketAddress & address);
		virtual int ReceiveFrom(void * buffer, int length, SocketAddress & address);
		virtual SocketAddress Address();
		virtual void SetAddress(const SocketAddress & address);

	protected:
		virtual ~DatagramSocketImpl();

		virtual void Init();

		static uv_udp_t * CreateUdpHandle(uv_loop_t * loop);

	private:
		SocketAddress address_;
	};
}

#endif
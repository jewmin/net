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

#ifndef Net_SocketImpl_INCLUDED
#define Net_SocketImpl_INCLUDED

#include "RefCountedObject.h"
#include "Address/SocketAddress.h"

namespace Net {
	class SocketImpl : public Foundation::RefCountedObject {
	public:
		SocketImpl(const SocketImpl &) = delete;
		SocketImpl & operator=(const SocketImpl &) = delete;

		virtual void Open(uv_loop_t * loop);
		virtual void Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free));
		virtual int Bind(const SocketAddress & address, bool ipV6Only = false, bool reuseAddress = false);
		virtual int Listen(int backlog, uv_connection_cb cb);
		virtual int Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb);
		virtual SocketImpl * AcceptConnection(SocketAddress & clientAddr);
		virtual void ShutdownReceive();
		virtual void ShutdownSend(uv_shutdown_cb cb = reinterpret_cast<uv_shutdown_cb>(free));
		virtual void Shutdown(uv_shutdown_cb cb = reinterpret_cast<uv_shutdown_cb>(free));
		virtual int Established(uv_alloc_cb allocCb, uv_read_cb readCb);
		virtual void SetSendBufferSize(int size);
		virtual int GetSendBufferSize();
		virtual void SetReceiveBufferSize(int size);
		virtual int GetReceiveBufferSize();
		virtual SocketAddress LocalAddress();
		virtual SocketAddress RemoteAddress();
		virtual void SetNoDelay();
		virtual void SetKeepAlive(int interval);
		virtual void Attach(uv_handle_t * handle);
		virtual uv_handle_t * Detatch();
		virtual uv_handle_t * GetHandle();

	protected:
		SocketImpl();
		virtual ~SocketImpl();

	private:
		uv_handle_t * handle_;
	};
}

inline void Net::SocketImpl::Attach(uv_handle_t * handle) {
	if (handle_) {
		throw std::invalid_argument("Socket already attached");
	}
	handle_ = handle;
}

inline uv_handle_t * Net::SocketImpl::Detatch() {
	uv_handle_t * handle = handle_;
	handle_ = nullptr;
	return handle;
}

inline uv_handle_t * Net::SocketImpl::GetHandle() {
	return handle_;
}

#endif
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

#ifndef Net_Sockets_SocketImpl_INCLUDED
#define Net_Sockets_SocketImpl_INCLUDED

#include "Common/RefCountedObject.h"
#include "Address/SocketAddress.h"
#include "Common/Logger.h"

namespace Net {

class SocketImpl : public RefCountedObject {
public:
	virtual ~SocketImpl();
	virtual void Open(uv_loop_t * loop);
	virtual void Close(uv_close_cb cb = FreeHandle);
	virtual i32 Bind(const SocketAddress & address, bool ipv6_only = false, bool reuse_address = false);
	virtual i32 Listen(i32 backlog, uv_connection_cb cb);
	virtual i32 Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb);
	virtual SocketImpl * AcceptConnection(SocketAddress & client_address);
	virtual void ShutdownReceive();
	virtual void ShutdownSend(uv_shutdown_t * req, uv_shutdown_cb cb);
	virtual void Shutdown(uv_shutdown_t * req, uv_shutdown_cb cb);
	virtual i32 Established(uv_alloc_cb alloc_cb, uv_read_cb read_cb);
	virtual i32 Send(const i8 * data, i32 len, uv_write_t * req, uv_write_cb cb);
	virtual void SetSendBufferSize(i32 size);
	virtual i32 GetSendBufferSize();
	virtual void SetReceiveBufferSize(i32 size);
	virtual i32 GetReceiveBufferSize();
	virtual SocketAddress LocalAddress();
	virtual SocketAddress RemoteAddress();
	virtual void SetNoDelay();
	virtual void SetKeepAlive(i32 interval);
	virtual void Attach(uv_handle_t * handle);
	virtual uv_handle_t * Detatch();
	virtual uv_handle_t * GetHandle();

	static void FreeHandle(uv_handle_t * handle);

protected:
	SocketImpl();

private:
	SocketImpl(SocketImpl &&) = delete;
	SocketImpl(const SocketImpl &) = delete;
	SocketImpl & operator=(SocketImpl &&) = delete;
	SocketImpl & operator=(const SocketImpl &) = delete;

private:
	uv_handle_t * handle_;
};

inline void SocketImpl::Attach(uv_handle_t * handle) {
	if (handle_) {
		Log(kCrash, __FILE__, __LINE__, "Socket already attached");
	}
	handle_ = handle;
}

inline uv_handle_t * SocketImpl::Detatch() {
	uv_handle_t * handle = handle_;
	handle_ = nullptr;
	return handle;
}

inline uv_handle_t * SocketImpl::GetHandle() {
	return handle_;
}

}

#endif
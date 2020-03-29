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
#include "Sockets/UvData.h"

namespace Net {

class SocketImpl : public RefCountedObject {
public:
	virtual ~SocketImpl();

	virtual void Open(uv_loop_t * loop);
	virtual void Close();
	virtual i32 Bind(const SocketAddress & address, bool ipv6_only = false, bool reuse_address = false);
	virtual i32 Listen(i32 backlog = 128);
	virtual i32 Connect(const SocketAddress & address, void * arg = nullptr);
	virtual SocketImpl * AcceptSocket(SocketAddress & client_address);
	virtual i32 Shutdown(void * arg = nullptr);
	virtual i32 ShutdownWrite(void * arg = nullptr);
	virtual i32 ShutdownRead();
	virtual i32 Established();
	virtual i32 Write(const i8 * data, i32 len, void * arg = nullptr);

	virtual void SetSendBufferSize(i32 size);
	virtual i32 GetSendBufferSize() const;
	virtual void SetRecvBufferSize(i32 size);
	virtual i32 GetRecvBufferSize() const;
	virtual SocketAddress LocalAddress();
	virtual SocketAddress RemoteAddress();
	virtual void SetNoDelay();
	virtual void SetKeepAlive(i32 interval);

	virtual void SetUvData(UvData * data);

protected:
	SocketImpl();

private:
	static void close_cb(uv_handle_t * handle);
	static void connection_cb(uv_stream_t * server, int status);
	static void connect_cb(uv_connect_t * req, int status);
	static void shutdown_cb(uv_shutdown_t * req, int status);
	static void alloc_cb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf);
	static void read_cb(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
	static void write_cb(uv_write_t * req, int status);

	SocketImpl(SocketImpl &&) = delete;
	SocketImpl(const SocketImpl &) = delete;
	SocketImpl & operator=(SocketImpl &&) = delete;
	SocketImpl & operator=(const SocketImpl &) = delete;

private:
	uv_handle_t * handle_;
};

}

#endif
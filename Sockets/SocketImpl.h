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
		static const size_t kBufferSize = 65536;

		SocketImpl() = delete;
		SocketImpl(const SocketImpl &) = delete;
		SocketImpl & operator=(const SocketImpl &) = delete;

		virtual SocketImpl * AcceptConnection(SocketAddress & clientAddr);
		virtual void Connect(const SocketAddress & address, const int timeout = 0);
		virtual void Bind(const SocketAddress & address, bool ipV6Only = false, bool reuseAddress = false);
		virtual void Listen(int backlog = 64);
		virtual void Close();
		virtual void ShutdownReceive();
		virtual void ShutdownSend();
		virtual void Shutdown();
		virtual void StartRead();
		virtual int SendBytes(const void * buffer, int length);
		virtual int ReceiveBytes(void * buffer, int length);
		virtual int SendTo(const void * buffer, int length, const SocketAddress & address);
		virtual int ReceiveFrom(void * buffer, int length, SocketAddress & address);
		virtual void SetSendBufferSize(int size);
		virtual int GetSendBufferSize();
		virtual void SetReceiveBufferSize(int size);
		virtual int GetReceiveBufferSize();
		virtual SocketAddress LocalAddress();
		virtual SocketAddress RemoteAddress();
		virtual void SetNoDelay(bool flag);
		virtual void SetKeepAlive(bool flag, int interval);

		template<typename T>
		T * Get() const;

	protected:
		SocketImpl(size_t maxSendBufferSize = kBufferSize, size_t maxReceiveBufferSize = kBufferSize);
		SocketImpl(uv_handle_t * handle, size_t maxSendBufferSize = kBufferSize, size_t maxReceiveBufferSize = kBufferSize);
		virtual ~SocketImpl();

		virtual void Init();
		void InitSocket(uv_handle_type type);

	private:
		uv_handle_t * handle_;
		uv_buf_t buffer_;
		size_t max_send_buffer_size_;
		size_t max_receive_buffer_size_;
		SocketAddress address_;

		static void CloseCallback(uv_handle_t * handle);
		static void ShutdownCallback(uv_shutdown_t * req, int status);
		static void ConnectionCallback(uv_stream_t * server, int status);
		static void ConnectCallback(uv_connect_t * req, int status);
		static void WriteCallback(uv_write_t * req, int status);
		static void AllocCallback(uv_handle_t * handle,	size_t suggested_size, uv_buf_t * buf);
		static void ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf);
		static void TimerCallback(uv_timer_t * handle);
		static void CloseTimerCallback(uv_handle_t * handle);
	};
}

template<typename T>
inline T * Net::SocketImpl::Get() const {
	return reinterpret_cast<T *>(handle_);
}

#endif
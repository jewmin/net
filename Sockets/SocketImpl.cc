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

#include "SocketImpl.h"
#include "StreamSocketImpl.h"

void Net::SocketImpl::CloseCallback(uv_handle_t * handle) {
	SocketImpl * impl = reinterpret_cast<SocketImpl *>(uv_handle_get_data(handle));
	if (impl) {
		impl->Release();
	}
	SafeFree(&handle);
}

void Net::SocketImpl::ShutdownCallback(uv_shutdown_t * req, int status) {
	SocketImpl * impl = reinterpret_cast<SocketImpl *>(req->handle->data);
	if (impl) {
		impl->Close();
	}
	SafeFree(&req);
}

void Net::SocketImpl::ConnectionCallback(uv_stream_t * server, int status) {
	if (0 == status) {

	}
}

void Net::SocketImpl::ConnectCallback(uv_connect_t * req, int status) {
	uv_timer_t * timer = reinterpret_cast<uv_timer_t *>(req->data);
	if (timer) {
		uv_close(reinterpret_cast<uv_handle_t *>(timer), CloseTimerCallback);
	}
	SafeFree(&req);
}

void Net::SocketImpl::WriteCallback(uv_write_t * req, int status) {
	if (0 != status) {
		SocketImpl * impl = reinterpret_cast<SocketImpl *>(req->handle->data);
		if (impl) {
			impl->Close();
		}
		Output("WriteCallback Error: %p - %s", impl, uv_strerror(status));
	}
	SafeFree(&req);
}

void Net::SocketImpl::AllocCallback(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
}

void Net::SocketImpl::ReadCallback(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf) {
}

void Net::SocketImpl::TimerCallback(uv_timer_t * handle) {
	uv_connect_t * req = reinterpret_cast<uv_connect_t *>(handle->data);
	if (req) {
		req->data = nullptr;
		SocketImpl * impl = reinterpret_cast<SocketImpl *>(req->handle->data);
		if (impl) {
			impl->Close();
		}
	}
	uv_close(reinterpret_cast<uv_handle_t *>(handle), CloseTimerCallback);
}

void Net::SocketImpl::CloseTimerCallback(uv_handle_t * handle) {
	SafeFree(&handle);
}

//*********************************************************************
//SocketImpl
//*********************************************************************

Net::SocketImpl::SocketImpl(size_t maxSendBufferSize, size_t maxReceiveBufferSize)
	: handle_(nullptr), buffer_(uv_buf_init(nullptr, 0))
	, max_send_buffer_size_(maxSendBufferSize), max_receive_buffer_size_(maxReceiveBufferSize) {
}

Net::SocketImpl::SocketImpl(uv_handle_t * handle, size_t maxSendBufferSize, size_t maxReceiveBufferSize)
	: handle_(handle), buffer_(uv_buf_init(nullptr, 0))
	, max_send_buffer_size_(maxSendBufferSize), max_receive_buffer_size_(maxReceiveBufferSize) {
	uv_handle_set_data(handle_, this);
	Duplicate();
}

Net::SocketImpl::~SocketImpl() {
	Close();
	SafeFree(&buffer_.base);
}

Net::SocketImpl * Net::SocketImpl::AcceptConnection(SocketAddress & clientAddr) {
	if (handle_ && UV_TCP == handle_->type) {
		uv_handle_t * client = SafeMalloc<uv_handle_t>(sizeof(uv_tcp_t));
		int err = uv_tcp_init(uv_default_loop(), reinterpret_cast<uv_tcp_t *>(client));
		if (0 == err) {
			err = uv_accept(Get<uv_stream_t>(), reinterpret_cast<uv_stream_t *>(client));
			if (0 == err) {
				struct sockaddr_storage buffer;
				struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
				socklen_t len = sizeof(buffer);
				err = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(client), sa, reinterpret_cast<int *>(&len));
				if (0 == err) {
					clientAddr = SocketAddress(sa, len);
					return new StreamSocketImpl(client, max_send_buffer_size_, max_receive_buffer_size_);
				}
			}
		}

		SafeFree(&client);
		Output("AcceptConnection Error: %p - %s", this, uv_strerror(err));
	}

	return nullptr;
}

void Net::SocketImpl::Connect(const SocketAddress & address, const int timeout) {
	if (!handle_) {
		Init();
	}

	if (handle_ && UV_TCP == handle_->type) {
		int err = 0;
		if (timeout > 0) {
			uv_timer_init(uv_default_loop(), timer_handle_);
			uv_timer_start(timer_handle_, TimerCallback, timeout, 0);
		}

		if (0 == err) {
			err = uv_tcp_connect(connect_request_, Get<uv_tcp_t>(), address.Addr(), ConnectCallback);
			if (0 == err) {

			}
			if (0 != err) {
				if (timer) {
					uv_close(reinterpret_cast<uv_handle_t *>(timer), CloseTimerCallback);
				}
				SafeFree(&req);
				Output("Connect Error: %p - %s", this, uv_strerror(err));
				return;
			}

			req->data = timer;
			if (timer) {
				timer->data = req;
			}
		}
	}
}

void Net::SocketImpl::Bind(const SocketAddress & address, bool ipV6Only, bool reuseAddress) {
	if (!handle_) {
		Init();
	}

	if (handle_ && UV_TCP == handle_->type) {
		int flags = 0;
		if (ipV6Only) {
			flags |= UV_TCP_IPV6ONLY;
		}
		int err = uv_tcp_bind(reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), flags);
		if (0 != err) {
			Output("Bind Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::Listen(int backlog) {
	if (handle_ && UV_TCP == handle_->type) {
		int err = uv_listen(reinterpret_cast<uv_stream_t *>(handle_), backlog, ConnectionCallback);
		if (0 != err) {
			Close();
			Output("Listen Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::Close() {
	if (handle_) {
		if (!uv_is_closing(handle_)) {
			uv_close(handle_, CloseCallback);
		}
		handle_ = nullptr;
	}
}

void Net::SocketImpl::ShutdownReceive() {
	if (handle_ && UV_TCP == handle_->type) {
		int err = uv_read_stop(reinterpret_cast<uv_stream_t *>(handle_));
		if (0 != err) {
			Output("ShutdownReceive Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::ShutdownSend() {
	if (handle_ && UV_TCP == handle_->type) {
		uv_shutdown_t * req = SafeMalloc<uv_shutdown_t>();
		int err = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(handle_), ShutdownCallback);
		if (0 != err) {
			SafeFree(&req);
			Close();
			Output("ShutdownSend Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::Shutdown() {
	ShutdownReceive();
	ShutdownSend();
}

void Net::SocketImpl::StartRead() {
	if (handle_ && UV_TCP == handle_->type) {
		int err = uv_read_start(reinterpret_cast<uv_stream_t *>(handle_), AllocCallback, ReadCallback);
		if (0 != err) {
			Close();
			Output("StartRead Error: %p - %s", this, uv_strerror(err));
		}
	}
}

int Net::SocketImpl::SendBytes(const void * buffer, int length) {
	if (!handle_ || UV_TCP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}

	int left_len = static_cast<int>(max_send_buffer_size_ - uv_stream_get_write_queue_size(Get<uv_stream_t>()));
	if (length > left_len) {
		length = left_len;
	}

	if (!buffer || length <= 0) {
		return UV_ENOBUFS;
	}

	uv_buf_t buf = uv_buf_init(static_cast<char *>(const_cast<void *>(buffer)), length);
	uv_write_t * req = SafeMalloc<uv_write_t>();
	int err = uv_write(req, Get<uv_stream_t>(), &buf, 1, WriteCallback);
	if (0 != err) {
		SafeFree(&req);
		Output("SendBytes Error: %p - %s", this, uv_strerror(err));
		return err;
	}

	return length;
}

int Net::SocketImpl::ReceiveBytes(void * buffer, int length) {
	if (!handle_ || UV_TCP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}

	if (!buffer || length <= 0 || !buffer_.base) {
		return UV_ENOBUFS;
	}

	if (static_cast<int>(buffer_.len) < length) {
		length = buffer_.len;
	}

	if (length > 0) {
		std::memcpy(buffer, buffer_.base, length);
		buffer_.len -= length;
		if (buffer_.len > 0) {
			std::memmove(buffer_.base, buffer_.base + length, buffer_.len);
		}
	}

	return length;
}

int Net::SocketImpl::SendTo(const void * buffer, int length, const SocketAddress & address) {
	if (!handle_ || UV_UDP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}


}

int Net::SocketImpl::ReceiveFrom(void * buffer, int length, SocketAddress & address) {
	if (!handle_ || UV_UDP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}

	if (!buffer || length <= 0 || !buffer_.base || static_cast<int>(buffer_.len) > length) {
		return UV_ENOBUFS;
	}

	length = buffer_.len;
	std::memcpy(buffer, buffer_.base, length);
	buffer_.len = 0;
	return length;
}

void Net::SocketImpl::SetSendBufferSize(int size) {
	max_send_buffer_size_ = size;
	if (handle_) {
		int err = uv_send_buffer_size(handle_, &size);
		if (0 != err) {
			Output("SetSendBufferSize Error: %p - %s", this, uv_strerror(err));
		}
	}
}

int Net::SocketImpl::GetSendBufferSize() {
	return max_send_buffer_size_;
}

void Net::SocketImpl::SetReceiveBufferSize(int size) {
	max_receive_buffer_size_ = size;
	if (handle_) {
		int err = uv_recv_buffer_size(handle_, &size);
		if (0 != err) {
			Output("SetReceiveBufferSize Error: %p - %s", this, uv_strerror(err));
		}
	}
}

int Net::SocketImpl::GetReceiveBufferSize() {
	return max_receive_buffer_size_;
}

Net::SocketAddress Net::SocketImpl::LocalAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		int err = uv_tcp_getsockname(Get<uv_tcp_t>(), sa, reinterpret_cast<int *>(&len));
		if (0 != err) {
			Output("LocalAddress Error: %p - %s", this, uv_strerror(err));
		} else {
			return SocketAddress(sa, len);
		}
	}

	return SocketAddress();
}

Net::SocketAddress Net::SocketImpl::RemoteAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		int err = uv_tcp_getpeername(Get<uv_tcp_t>(), sa, reinterpret_cast<int *>(&len));
		if (0 != err) {
			Output("RemoteAddress Error: %p - %s", this, uv_strerror(err));
		} else {
			return SocketAddress(sa, len);
		}
	}

	return SocketAddress();
}

void Net::SocketImpl::SetNoDelay(bool flag) {
	if (handle_ && UV_TCP == handle_->type) {
		int enable = flag ? 1 : 0;
		int err = uv_tcp_nodelay(Get<uv_tcp_t>(), enable);
		if (0 != err) {
			Output("SetNoDelay Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::SetKeepAlive(bool flag, int interval) {
	if (handle_ && UV_TCP == handle_->type) {
		int enable = flag ? 1 : 0;
		int err = uv_tcp_keepalive(Get<uv_tcp_t>(), enable, interval);
		if (0 != err) {
			Output("SetKeepAlive Error: %p - %s", this, uv_strerror(err));
		}
	}
}

void Net::SocketImpl::Init() {
	InitSocket(UV_TCP);
}

void Net::SocketImpl::InitSocket(uv_handle_type type) {
	if (!handle_) {
		if (UV_TCP == type) {
			handle_ = SafeMalloc<uv_handle_t>(sizeof(uv_tcp_t));
			uv_tcp_init(uv_default_loop(), Get<uv_tcp_t>());
		} else if (UV_UDP == type) {
			handle_ = SafeMalloc<uv_handle_t>(sizeof(uv_udp_t));
			uv_udp_init(uv_default_loop(), Get<uv_udp_t>());
		}

		if (handle_) {
			uv_handle_set_data(handle_, this);
			Duplicate();
		}
	}
}
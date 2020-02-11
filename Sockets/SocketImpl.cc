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

#include "Allocator.h"
#include "SocketImpl.h"
#include "StreamSocketImpl.h"

namespace Net {

SocketImpl::SocketImpl() : handle_(nullptr) {
}

SocketImpl::~SocketImpl() {
	Close();
}

void SocketImpl::Open(uv_loop_t * loop) {
	if (!handle_) {
		handle_ = static_cast<uv_handle_t *>(Allocator::Get()->Allocate(sizeof(uv_tcp_t)));
		uv_tcp_init(loop, reinterpret_cast<uv_tcp_t *>(handle_));
		handle_->data = nullptr;
	}
}

void SocketImpl::Close(uv_close_cb cb) {
	if (handle_) {
		if (!uv_is_closing(handle_)) {
			uv_close(handle_, cb);
		}
		handle_ = nullptr;
	}
}

void SocketImpl::FreeHandle(uv_handle_t * handle) {
	if (handle) {
		if (UV_TCP == handle->type) {
			Allocator::Get()->DeAllocate(handle, sizeof(uv_tcp_t));
		} else {
			Log(kCrash, __FILE__, __LINE__, "释放内存失败", uv_handle_type_name(handle->type));
		}
	}
}

i32 SocketImpl::Bind(const SocketAddress & address, bool ipv6_only, bool reuse_address) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		i32 flags = 0;
		if (ipv6_only) {
			flags |= UV_TCP_IPV6ONLY;
		}
		status = uv_tcp_bind(reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), flags);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "绑定端口失败", address.ToString().c_str(), uv_strerror(status));
			Close();
		}
	}
	return status;
}

i32 SocketImpl::Listen(i32 backlog, uv_connection_cb cb) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_listen(reinterpret_cast<uv_stream_t *>(handle_), backlog, cb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "监听端口失败", uv_strerror(status));
			Close();
		}
	}
	return status;
}

i32 SocketImpl::Connect(const SocketAddress & address, uv_connect_t * req, uv_connect_cb cb) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_tcp_connect(req, reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), cb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "连接远端失败", address.ToString().c_str(), uv_strerror(status));
			Close();
		}
	}
	return status;
}

SocketImpl * SocketImpl::AcceptConnection(SocketAddress & client_address) {
	if (handle_ && UV_TCP == handle_->type) {
		SocketImpl * client = new StreamSocketImpl();
		client->Open(handle_->loop);
		i32 status = uv_accept(reinterpret_cast<uv_stream_t *>(handle_), reinterpret_cast<uv_stream_t *>(client->GetHandle()));
		if (0 == status) {
			struct sockaddr_storage buffer;
			struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
			socklen_t len = sizeof(buffer);
			status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(client->GetHandle()), sa, reinterpret_cast<i32 *>(&len));
			if (0 == status) {
				client_address = SocketAddress(sa, len);
				return client;
			}
		}
		Log(kLog, __FILE__, __LINE__, "接受连接失败", uv_strerror(status));
		client->Release();
	}
	return nullptr;
}

void SocketImpl::ShutdownReceive() {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_read_stop(reinterpret_cast<uv_stream_t *>(handle_));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "关闭接收端失败", uv_strerror(status));
		}
	}
}

void SocketImpl::ShutdownSend(uv_shutdown_t * req, uv_shutdown_cb cb) {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(handle_), cb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "关闭发送端失败", uv_strerror(status));
		}
	}
}

void SocketImpl::Shutdown(uv_shutdown_t * req, uv_shutdown_cb cb) {
	ShutdownReceive();
	ShutdownSend(req, cb);
}

i32 SocketImpl::Established(uv_alloc_cb allocCb, uv_read_cb readCb) {
	i32 status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_read_start(reinterpret_cast<uv_stream_t *>(handle_), allocCb, readCb);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "开始读数据失败", uv_strerror(status));
		}
	}
	return status;
}

i32 SocketImpl::Send(const i8 * data, i32 len, uv_write_t * req, uv_write_cb cb) {
	if (!handle_ || UV_TCP != handle_->type) {
		return UV_EPROTONOSUPPORT;
	}
	if (!data || len <= 0) {
		return UV_ENOBUFS;
	}
	uv_buf_t buf = uv_buf_init(const_cast<i8 *>(data), len);
	i32 status = uv_write(req, reinterpret_cast<uv_stream_t * >(handle_), &buf, 1, cb);
	if (status < 0) {
		Log(kLog, __FILE__, __LINE__, "发送数据失败", uv_strerror(status));
		return status;
	}
	return len;
}

void SocketImpl::SetSendBufferSize(i32 size) {
	if (handle_) {
		i32 status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "设置发送缓冲大小失败", uv_strerror(status));
		}
	}
}

i32 SocketImpl::GetSendBufferSize() {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "获取发送缓冲大小失败", uv_strerror(status));
		}
	}
	return size;
}

void SocketImpl::SetReceiveBufferSize(i32 size) {
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "设置接收缓冲大小失败", uv_strerror(status));
		}
	}
}

i32 SocketImpl::GetReceiveBufferSize() {
	i32 size = 0;
	if (handle_) {
		i32 status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "获取接收缓冲大小失败", uv_strerror(status));
		}
	}
	return size;
}

SocketAddress SocketImpl::LocalAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getsockname(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "获取本地地址失败", uv_strerror(status));
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

SocketAddress SocketImpl::RemoteAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		i32 status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<i32 *>(&len));
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "获取远端地址失败", uv_strerror(status));
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

void SocketImpl::SetNoDelay() {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_tcp_nodelay(reinterpret_cast<uv_tcp_t *>(handle_), 1);
		status = uv_translate_sys_error(status);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "禁止Negale算法失败", uv_strerror(status));
		}
	}
}

void SocketImpl::SetKeepAlive(i32 interval) {
	if (handle_ && UV_TCP == handle_->type) {
		i32 status = uv_tcp_keepalive(reinterpret_cast<uv_tcp_t *>(handle_), 1, interval);
		status = uv_translate_sys_error(status);
		if (status < 0) {
			Log(kLog, __FILE__, __LINE__, "设置KeepAlive机制失败", uv_strerror(status));
		}
	}
}

}
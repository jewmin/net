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

Net::SocketImpl::SocketImpl() : handle_(nullptr) {
}

Net::SocketImpl::~SocketImpl() {
	Close();
}

void Net::SocketImpl::Open(uv_loop_t * loop) {
	if (!handle_) {
		handle_ = static_cast<uv_handle_t *>(malloc(sizeof(uv_tcp_t)));
		uv_tcp_init(loop, reinterpret_cast<uv_tcp_t *>(handle_));
		handle_->data = nullptr;
	}
}

void Net::SocketImpl::Close(uv_close_cb cb) {
	if (handle_) {
		if (!uv_is_closing(handle_)) {
			uv_close(handle_, cb);
		}
		handle_ = nullptr;
	}
}

int Net::SocketImpl::Bind(const SocketAddress & address, bool ipV6Only, bool reuseAddress) {
	int status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		int flags = 0;
		if (ipV6Only) {
			flags |= UV_TCP_IPV6ONLY;
		}
		status = uv_tcp_bind(reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), flags);
		if (status < 0) {
			LogErr("绑定端口(%s)失败. %s", address.ToString().c_str(), uv_strerror(status));
			Close();
		}
	}
	return status;
}

int Net::SocketImpl::Listen(int backlog, uv_connection_cb cb) {
	int status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_listen(reinterpret_cast<uv_stream_t *>(handle_), backlog, cb);
		if (status < 0) {
			LogErr("监听端口失败. %s", uv_strerror(status));
			Close();
		}
	}
	return status;
}

int Net::SocketImpl::Connect(const SocketAddress & address, void * ud, uv_connect_cb cb) {
	int status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		uv_connect_t * req = static_cast<uv_connect_t *>(malloc(sizeof(uv_connect_t)));
		status = uv_tcp_connect(req, reinterpret_cast<uv_tcp_t *>(handle_), address.Addr(), cb);
		if (status < 0) {
			free(req);
			LogErr("连接远端(%s)失败. %s", address.ToString().c_str(), uv_strerror(status));
			Close();
		} else {
			req->data = ud;
		}
	}
	return status;
}

Net::SocketImpl * Net::SocketImpl::AcceptConnection(SocketAddress & clientAddr) {
	if (handle_ && UV_TCP == handle_->type) {
		SocketImpl * client = new StreamSocketImpl();
		client->Open(handle_->loop);
		int status = uv_accept(reinterpret_cast<uv_stream_t *>(handle_), reinterpret_cast<uv_stream_t *>(client->GetHandle()));
		if (0 == status) {
			struct sockaddr_storage buffer;
			struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
			socklen_t len = sizeof(buffer);
			status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(client->GetHandle()), sa, reinterpret_cast<int *>(&len));
			if (0 == status) {
				clientAddr = SocketAddress(sa, len);
				return client;
			}
		}
		LogErr("接受连接失败. %s", uv_strerror(status));
		client->Release();
	}
	return nullptr;
}

void Net::SocketImpl::ShutdownReceive() {
	if (handle_ && UV_TCP == handle_->type) {
		int status = uv_read_stop(reinterpret_cast<uv_stream_t *>(handle_));
		if (status < 0) {
			LogErr("关闭接收端失败. %s", uv_strerror(status));
		}
	}
}

void Net::SocketImpl::ShutdownSend(uv_shutdown_cb cb) {
	if (handle_ && UV_TCP == handle_->type) {
		uv_shutdown_t * req = static_cast<uv_shutdown_t *>(malloc(sizeof(uv_shutdown_t)));
		int status = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(handle_), cb);
		if (status < 0) {
			free(req);
			LogErr("关闭发送端失败. %s", uv_strerror(status));
		}
	}
}

void Net::SocketImpl::Shutdown(uv_shutdown_cb cb) {
	ShutdownReceive();
	ShutdownSend(cb);
}

int Net::SocketImpl::Established(uv_alloc_cb allocCb, uv_read_cb readCb) {
	int status = UV_UNKNOWN;
	if (handle_ && UV_TCP == handle_->type) {
		status = uv_read_start(reinterpret_cast<uv_stream_t *>(handle_), allocCb, readCb);
		if (status < 0) {
			LogErr("开始读数据失败. %s", uv_strerror(status));
		}
	}
	return status;
}

void Net::SocketImpl::SetSendBufferSize(int size) {
	if (handle_) {
		int status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			LogErr("设置发送缓冲大小失败. %s", uv_strerror(status));
		}
	}
}

int Net::SocketImpl::GetSendBufferSize() {
	int size = 0;
	if (handle_) {
		int status = uv_send_buffer_size(handle_, &size);
		if (status < 0) {
			LogErr("获取发送缓冲大小失败. %s", uv_strerror(status));
		}
	}
	return size;
}

void Net::SocketImpl::SetReceiveBufferSize(int size) {
	if (handle_) {
		int status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			LogErr("设置接收缓冲大小失败. %s", uv_strerror(status));
		}
	}
}

int Net::SocketImpl::GetReceiveBufferSize() {
	int size = 0;
	if (handle_) {
		int status = uv_recv_buffer_size(handle_, &size);
		if (status < 0) {
			LogErr("获取接收缓冲大小失败. %s", uv_strerror(status));
		}
	}
	return size;
}

Net::SocketAddress Net::SocketImpl::LocalAddress() {
	if (handle_ && UV_TCP == handle_->type) {
		struct sockaddr_storage buffer;
		struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
		socklen_t len = sizeof(buffer);
		int status = uv_tcp_getsockname(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<int *>(&len));
		if (status < 0) {
			LogErr("获取本地地址失败. %s", uv_strerror(status));
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
		int status = uv_tcp_getpeername(reinterpret_cast<uv_tcp_t *>(handle_), sa, reinterpret_cast<int *>(&len));
		if (status < 0) {
			LogErr("获取远端地址失败. %s", uv_strerror(status));
		} else {
			return SocketAddress(sa, len);
		}
	}
	return SocketAddress();
}

void Net::SocketImpl::SetNoDelay() {
	if (handle_ && UV_TCP == handle_->type) {
		int status = uv_tcp_nodelay(reinterpret_cast<uv_tcp_t *>(handle_), 1);
		if (status < 0) {
			LogErr("禁止Negale算法失败. %s", uv_strerror(status));
		}
	}
}

void Net::SocketImpl::SetKeepAlive(int interval) {
	if (handle_ && UV_TCP == handle_->type) {
		int status = uv_tcp_keepalive(reinterpret_cast<uv_tcp_t *>(handle_), 1, interval);
		if (status < 0) {
			LogErr("设置KeepAlive机制失败. %s", uv_strerror(status));
		}
	}
}
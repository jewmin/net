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

#include "DatagramSocketImpl.h"
#include "Socket.h"
#include "Reactor/SocketReactor.h"

void udp_alloc_cb(uv_handle_t * handle, size_t suggested_size, uv_buf_t * buf) {
	Net::Socket * socket = static_cast<Net::Socket *>(handle->data);
	if (socket) {
		uv_buf_t buffer = socket->GetBuffer();
		buf->base = buffer.base;
		buf->len = Net::SocketImpl::kBufferSize;
	}
}

void udp_recv_cb(uv_udp_t * handle, ssize_t nread, const uv_buf_t * buf, const struct sockaddr * addr, unsigned flags) {
	Net::SocketReactor * reactor = static_cast<Net::SocketReactor *>(handle->loop->data);
	Net::Socket * socket = static_cast<Net::Socket *>(handle->data);
	if (reactor && socket) {
		Net::SocketNotification * notification = nullptr;
		if (nread >= 0) {
			uv_buf_t buffer = socket->GetBuffer();
			buffer.len = nread;
			if (AF_INET == addr->sa_family) {
				socket->SetAddress(Net::SocketAddress(addr, sizeof(struct sockaddr_in)));
			} else if (AF_INET6 == addr->sa_family) {
				socket->SetAddress(Net::SocketAddress(addr, sizeof(struct sockaddr_in6)));
			} else {
				socket->SetAddress(Net::SocketAddress());
			}
			notification = reactor->GetNotification(Net::SocketNotification::kRead);
		} else {
			notification = reactor->GetNotification(Net::SocketNotification::kError);
		}
		reactor->Dispatch(*socket, notification);
	}
}

Net::DatagramSocketImpl::DatagramSocketImpl(uv_loop_t * loop)
	: SocketImpl(loop_) {
}

void Net::DatagramSocketImpl::Bind(const SocketAddress & address, bool reuseAddress, bool ipV6Only) {
	if (nullptr == handle_) {
		Init();
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("Bind(): handle_->type is not UV_UDP");
	}

	int flags = 0;
	if (reuseAddress) {
		flags |= UV_UDP_REUSEADDR;
	}
	if (ipV6Only) {
		flags |= UV_UDP_IPV6ONLY;
	}

	last_error_ = uv_udp_bind(reinterpret_cast<uv_udp_t *>(handle_), address.Addr(), flags);
}

void Net::DatagramSocketImpl::ShutdownReceive() {
	if (nullptr == handle_) {
		throw std::invalid_argument("ShutdownReceive(): handle_ is null");
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("ShutdownReceive(): handle_->type is not UV_UDP");
	}

	last_error_ = uv_udp_recv_stop(reinterpret_cast<uv_udp_t *>(handle_));
}

void Net::DatagramSocketImpl::StartRead() {
	if (nullptr == handle_) {
		throw std::invalid_argument("StartRead(): handle_ is null");
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("StartRead(): handle_->type is not UV_UDP");
	}

	last_error_ = uv_udp_recv_start(reinterpret_cast<uv_udp_t *>(handle_), udp_alloc_cb, udp_recv_cb);
}

int Net::DatagramSocketImpl::SendTo(const void * buffer, int length, const SocketAddress & address) {
	if (nullptr == handle_) {
		Init();
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("SendTo(): handle_->type is not UV_UDP");
	}

	uv_buf_t buf = uv_buf_init(static_cast<char *>(const_cast<void *>(buffer)), length);
	return uv_udp_try_send(reinterpret_cast<uv_udp_t *>(handle_), &buf, 1, address.Addr());
}

int Net::DatagramSocketImpl::ReceiveFrom(void * buffer, int length, SocketAddress & address) {
	if (nullptr == handle_) {
		throw std::invalid_argument("SendTo(): handle_ is null");
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("SendTo(): handle_->type is not UV_UDP");
	}

	if (length <= 0 || static_cast<int>(buffer_.len) > length) {
		return UV_ENOBUFS;
	}

	address = address_;
	std::memcpy(buffer, buffer_.base, buffer_.len);
	return buffer_.len;
}

Net::SocketAddress Net::DatagramSocketImpl::Address() {
	if (nullptr == handle_) {
		throw std::invalid_argument("Address(): handle_ is null");
	}

	if (UV_UDP != handle_->type) {
		throw std::invalid_argument("Address(): handle_->type is not UV_UDP");
	}

	struct sockaddr_storage buffer;
	struct sockaddr * sa = reinterpret_cast<struct sockaddr *>(&buffer);
	socklen_t saLen = sizeof(buffer);
	last_error_ = uv_udp_getsockname(reinterpret_cast<uv_udp_t *>(handle_), sa, reinterpret_cast<int *>(&saLen));
	if (0 == last_error_) {
		return SocketAddress(sa, saLen);
	}

	return SocketAddress();
}

void Net::DatagramSocketImpl::SetAddress(const SocketAddress & address) {
	address_ = address;
}

Net::DatagramSocketImpl::~DatagramSocketImpl() {
}

void Net::DatagramSocketImpl::Init() {
	if (nullptr != handle_) {
		throw std::invalid_argument("Init(): handle_ is not null");
	}

	handle_ = reinterpret_cast<uv_handle_t *>(CreateUdpHandle(loop_));
	if (nullptr == handle_) {
		throw std::invalid_argument("Init(): handle_ is null");
	}
}

uv_udp_t * Net::DatagramSocketImpl::CreateUdpHandle(uv_loop_t * loop) {
	uv_udp_t * udp = static_cast<uv_udp_t *>(::malloc(sizeof(uv_udp_t)));
	last_error_ = uv_udp_init(loop, udp);
	if (0 == last_error_) {
		return udp;
	}

	::free(udp);
	return nullptr;
}
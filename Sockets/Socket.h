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

#ifndef Net_Socket_INCLUDED
#define Net_Socket_INCLUDED

#include "SocketImpl.h"

namespace Net {
	class Socket {
	public:
		Socket();
		Socket(const Socket & rhs);
		Socket & operator=(const Socket & rhs);
		virtual ~Socket();
		
		void Open(uv_loop_t * loop);
		void Close(uv_close_cb cb = reinterpret_cast<uv_close_cb>(free));
		void SetSendBufferSize(int size);
		int GetSendBufferSize() const;
		void SetReceiveBufferSize(int size);
		int GetReceiveBufferSize() const;
		SocketAddress LocalAddress();
		SocketAddress RemoteAddress();
		void SetNoDelay();
		void SetKeepAlive(int interval);
		void SetHandle(uv_handle_t * handle);
		uv_handle_t * GetHandle();
		SocketImpl * Impl() const;

		bool operator==(const Socket & socket) const;
		bool operator!=(const Socket & socket) const;
		bool operator<(const Socket & socket) const;
		bool operator<=(const Socket & socket) const;
		bool operator>(const Socket & socket) const;
		bool operator>=(const Socket & socket) const;

	protected:
		explicit Socket(SocketImpl * impl);

	private:
		SocketImpl * impl_;
	};
}

inline void Net::Socket::Open(uv_loop_t * loop) {
	impl_->Open(loop);
}

inline void Net::Socket::Close(uv_close_cb cb) {
	impl_->Close(cb);
}

inline void Net::Socket::SetSendBufferSize(int size) {
	impl_->SetSendBufferSize(size);
}

inline int Net::Socket::GetSendBufferSize() const {
	return impl_->GetSendBufferSize();
}

inline void Net::Socket::SetReceiveBufferSize(int size) {
	impl_->SetReceiveBufferSize(size);
}

inline int Net::Socket::GetReceiveBufferSize() const {
	return impl_->GetReceiveBufferSize();
}

inline Net::SocketAddress Net::Socket::LocalAddress() {
	return impl_->LocalAddress();
}

inline Net::SocketAddress Net::Socket::RemoteAddress() {
	return impl_->RemoteAddress();
}

inline void Net::Socket::SetNoDelay() {
	impl_->SetNoDelay();
}

inline void Net::Socket::SetKeepAlive(int interval) {
	impl_->SetKeepAlive(interval);
}

inline void Net::Socket::SetHandle(uv_handle_t * handle) {
	impl_->SetHandle(handle);
}

inline uv_handle_t * Net::Socket::GetHandle() {
	return impl_->GetHandle();
}

inline Net::SocketImpl * Net::Socket::Impl() const {
	return impl_;
}

inline bool Net::Socket::operator==(const Socket & socket) const {
	return impl_ == socket.impl_;
}

inline bool Net::Socket::operator!=(const Socket & socket) const {
	return impl_ != socket.impl_;
}

inline bool Net::Socket::operator<(const Socket & socket) const {
	return impl_ < socket.impl_;
}

inline bool Net::Socket::operator<=(const Socket & socket) const {
	return impl_ <= socket.impl_;
}

inline bool Net::Socket::operator>(const Socket & socket) const {
	return impl_ > socket.impl_;
}

inline bool Net::Socket::operator>=(const Socket & socket) const {
	return impl_ >= socket.impl_;
}

#endif
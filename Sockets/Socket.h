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
		Socket(const Socket & rhs);
		Socket & operator=(const Socket & rhs);
		virtual ~Socket();
		
		void Close();
		void StartRead();
		void SetSendBufferSize(int size);
		int GetSendBufferSize() const;
		void SetReceiveBufferSize(int size);
		int GetReceiveBufferSize() const;
		SocketAddress Address() const;
		void SetAddress(const SocketAddress & address);
		uv_buf_t GetBuffer();
		SocketImpl * Impl() const;

		bool operator==(const Socket & socket) const;
		bool operator!=(const Socket & socket) const;
		bool operator<(const Socket & socket) const;
		bool operator<=(const Socket & socket) const;
		bool operator>(const Socket & socket) const;
		bool operator>=(const Socket & socket) const;

	protected:
		Socket(SocketImpl * impl);

	private:
		SocketImpl * impl_;
	};
}

inline void Net::Socket::Close() {
	impl_->Close();
}

inline void Net::Socket::StartRead() {
	impl_->StartRead();
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

inline Net::SocketAddress Net::Socket::Address() const {
	return impl_->Address();
}

inline void Net::Socket::SetAddress(const SocketAddress & address) {
	impl_->SetAddress(address);
}

inline uv_buf_t Net::Socket::GetBuffer() {
	return impl_->GetBuffer();
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
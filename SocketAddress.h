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

#ifndef Net_SocketAddress_INCLUDE
#define Net_SocketAddress_INCLUDE

#include "SocketAddressImpl.h"

namespace Net {
	class SocketAddress {
	public:
		SocketAddress();
		explicit SocketAddress(AddressFamily::Family family);
		SocketAddress(AddressFamily::Family family, u16 port);
		SocketAddress(AddressFamily::Family family, const std::string & hostAddress, u16 port);
		SocketAddress(AddressFamily::Family family, const std::string & hostAddress, const std::string & port);
		SocketAddress(AddressFamily::Family family, const std::string & addr);
		explicit SocketAddress(u16 port);
		SocketAddress(const IPAddress & hostAddress, u16 port);
		SocketAddress(const std::string & hostAddress, u16 port);
		SocketAddress(const std::string & hostAddress, const std::string & port);
		explicit SocketAddress(const std::string & hostAndPort);
		SocketAddress(const struct sockaddr * addr, socklen_t length);
		SocketAddress(const SocketAddress & rhs);
		SocketAddress & operator=(const SocketAddress & rhs);
		~SocketAddress();

		IPAddress Host() const;
		u16 Port() const;
		socklen_t Length() const;
		const struct sockaddr * Addr() const;
		int AF() const;
		AddressFamily::Family Family() const;
		std::string ToString() const;

		bool operator==(const SocketAddress & socketAddress) const;
		bool operator!=(const SocketAddress & socketAddress) const;

		static const AddressFamily::Family IPv4 = AddressFamily::IPv4;
		static const AddressFamily::Family IPv6 = AddressFamily::IPv6;

	protected:
		void Init(const IPAddress & hostAddress, u16 port);
		void Init(const std::string & hostAddress, u16 port);
		void Init(AddressFamily::Family family, const std::string & hostAddress, u16 port);

	private:
		SocketAddressImpl * Impl() const;

		void NewIPv4();
		void NewIPv4(const struct sockaddr_in * addr);
		void NewIPv4(const IPAddress & hostAddress, u16 port);
		void NewIPv6(const struct sockaddr_in6 * addr);
		void NewIPv6(const IPAddress & hostAddress, u16 port);
		void Destroy();
		char * Storage();

		static const u32 kSize = sizeof(IPv6SocketAddressImpl);
		union {
			char buffer[kSize];
		private:
			std::aligned_storage<kSize>::type aligner;
		} memory_;
	};
}

inline Net::IPAddress Net::SocketAddress::Host() const {
	return Impl()->Host();
}

inline Net::u16 Net::SocketAddress::Port() const {
	return Impl()->Port();
}

inline socklen_t Net::SocketAddress::Length() const {
	return Impl()->Length();
}

inline const sockaddr * Net::SocketAddress::Addr() const {
	return Impl()->Addr();
}

inline int Net::SocketAddress::AF() const {
	return Impl()->AF();
}

inline Net::AddressFamily::Family Net::SocketAddress::Family() const {
	return Impl()->Family();
}

inline std::string Net::SocketAddress::ToString() const {
	return Impl()->ToString();
}

Net::SocketAddressImpl * Net::SocketAddress::Impl() const {
	return reinterpret_cast<SocketAddressImpl *>(const_cast<char *>(memory_.buffer));
}

char * Net::SocketAddress::Storage() {
	return memory_.buffer;
}

#endif
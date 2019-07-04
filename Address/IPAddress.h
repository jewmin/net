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

#ifndef Net_IPAddress_INCLUDE
#define Net_IPAddress_INCLUDE

#include "IPAddressImpl.h"

namespace Net {
	class IPAddress {
	public:
		IPAddress();
		explicit IPAddress(AddressFamily::Family family);
		explicit IPAddress(const std::string & addr);
		IPAddress(const std::string & addr, AddressFamily::Family family);
		IPAddress(const void * addr, socklen_t length, u32 scope = 0);
		explicit IPAddress(const struct sockaddr & sockaddr);
		IPAddress(const IPAddress & rhs);
		IPAddress & operator=(const IPAddress & rhs);
		~IPAddress();
		
		std::string ToString() const;
		socklen_t Length() const;
		const void * Addr() const;
		AddressFamily::Family Family() const;
		int AF() const;
		u32 Scope() const;

		static IPAddress Parse(const std::string & addr);
		static bool TryParse(const std::string & addr, IPAddress & result);
		bool operator==(const IPAddress & addr) const;
		bool operator!=(const IPAddress & addr) const;

		static const AddressFamily::Family IPv4 = AddressFamily::IPv4;
		static const AddressFamily::Family IPv6 = AddressFamily::IPv6;

	private:
		IPAddressImpl * Impl() const;
		void NewIPv4();
		void NewIPv4(const void * host);
		void NewIPv6();
		void NewIPv6(const void * host, u32 scope);
		void Destroy();
		char * Storage();

		static const u32 kSize = sizeof(IPv6AddressImpl);
		union {
			char buffer[kSize];
		private:
			std::aligned_storage<kSize>::type aligner;
		} memory_;
	};
}

inline std::string Net::IPAddress::ToString() const {
	return Impl()->ToString();
}

inline socklen_t Net::IPAddress::Length() const {
	return Impl()->Length();
}

inline const void * Net::IPAddress::Addr() const {
	return Impl()->Addr();
}

inline Net::AddressFamily::Family Net::IPAddress::Family() const {
	return Impl()->Family();
}

inline int Net::IPAddress::AF() const {
	return Impl()->AF();
}

inline u32 Net::IPAddress::Scope() const {
	return Impl()->Scope();
}

inline Net::IPAddressImpl * Net::IPAddress::Impl() const {
	return reinterpret_cast<IPAddressImpl *>(const_cast<char *>(memory_.buffer));
}

inline char * Net::IPAddress::Storage() {
	return memory_.buffer;
}

#endif
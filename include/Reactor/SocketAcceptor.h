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

#ifndef Net_Reactor_SocketAcceptor_INCLUDED
#define Net_Reactor_SocketAcceptor_INCLUDED

#include "Reactor/EventHandler.h"
#include "Address/SocketAddress.h"
#include "Sockets/ServerSocket.h"

namespace Net {

class SocketConnection;
class COMMON_EXTERN SocketAcceptor : public EventHandler {
public:
	virtual ~SocketAcceptor();

	bool Open(const SocketAddress & address, i32 backlog = 128, bool ipv6_only = false);
	void Close();
	SocketAddress GetListenAddress() const;

protected:
	explicit SocketAcceptor(EventReactor * reactor);
	virtual bool RegisterToReactor() override;
	virtual bool UnRegisterFromReactor() override;
	virtual SocketConnection * CreateConnection() = 0;
	virtual void DestroyConnection(SocketConnection * connection) = 0;
	virtual void AcceptCallback(i32 status) override;

private:
	bool ActivateConnection(SocketConnection * connection);

private:
	bool opened_;
	ServerSocket socket_;
	SocketAddress address_;
};

inline SocketAddress SocketAcceptor::GetListenAddress() const {
	return address_;
}

}

#endif
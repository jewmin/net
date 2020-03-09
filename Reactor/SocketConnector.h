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

#ifndef Net_Reactor_SocketConnector_INCLUDED
#define Net_Reactor_SocketConnector_INCLUDED

#include "Reactor/EventHandler.h"
#include "Reactor/SocketConnection.h"
#include "Common/NetObject.h"

namespace Net {

class SocketConnector : public EventHandler {
	class Context : public NetObject {
	public:
		Context(SocketConnector * connector, SocketConnection * connection) : connector_(connector), connection_(connection) {}
		virtual ~Context() {}
		SocketConnector * connector_;
		SocketConnection * connection_;
	};

public:
	SocketConnector(EventReactor * reactor);
	virtual ~SocketConnector();

	i32 Connect(SocketConnection * connection, const SocketAddress & address);
	virtual void Destroy();

protected:
	virtual bool RegisterToReactor();
	virtual bool UnRegisterFromReactor();

private:
	void OnOneConnectSuccess(SocketConnection * connection);
	bool ActivateConnection(SocketConnection * connection);

	static void ConnectCb(uv_connect_t * req, i32 status);
};

}

#endif
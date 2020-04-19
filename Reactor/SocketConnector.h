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

namespace Net {

class SocketConnector : public EventHandler {
	class Context : public UvData {
	public:
		Context(SocketConnector * connector, SocketConnection * connection);
		virtual ~Context();
		virtual void ConnectCallback(i32 status, void * arg) override;
		SocketConnector * connector_;
		SocketConnection * connection_;
	};

public:
	explicit SocketConnector(EventReactor * reactor);
	virtual ~SocketConnector();

	bool Connect(SocketConnection * connection, const SocketAddress & address);

protected:
	virtual bool RegisterToReactor() override;
	virtual bool UnRegisterFromReactor() override;

private:
	bool ActivateConnection(SocketConnection * connection);
};

}

#endif
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

#ifndef Net_Interface_Server_INCLUDED
#define Net_Interface_Server_INCLUDED

#include "Interface/ConnectionMgr.h"
#include "Reactor/EventReactor.h"
#include "Reactor/SocketAcceptor.h"

namespace Net {

class Server : public ConnectionMgr {
	class Acceptor : public SocketAcceptor {
	public:
		Acceptor(EventReactor * reactor, Server * server);
		virtual ~Acceptor();

	protected:
		virtual SocketConnection * CreateConnection() override;
		virtual void DestroyConnection(SocketConnection * connection) override;

	private:
		Server * server_;
	};

public:
	Server(const std::string & name, EventReactor * reactor, i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~Server();

	virtual bool Listen(const std::string & address, i32 port, i32 backlog = 128, bool ipv6_only = false);
	virtual void Stop();

private:
	SocketAcceptor * acceptor_;
	i32 max_out_buffer_size_;
	i32 max_in_buffer_size_;
};

}

#endif
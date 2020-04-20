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

#include "Core/Server.h"
#include "Core/Connection.h"

namespace Net {

Server::Acceptor::Acceptor(EventReactor * reactor, Server * server)
	: SocketAcceptor(reactor), server_(server) {
}

Server::Acceptor::~Acceptor() {
}

SocketConnection * Server::Acceptor::CreateConnection() {
	return new Connection(server_, server_->max_out_buffer_size_, server_->max_in_buffer_size_);
}

Server::Server(const std::string & name, EventReactor * reactor, i32 max_out_buffer_size, i32 max_in_buffer_size, u32 object_max_count)
	: ConnectionMgr(name, object_max_count), acceptor_(new Acceptor(reactor, this))
	, max_out_buffer_size_(max_out_buffer_size), max_in_buffer_size_(max_in_buffer_size) {
}

Server::~Server() {
	if (acceptor_) {
		acceptor_->Destroy();
	}
}

bool Server::Listen(const std::string & address, i32 port, i32 backlog, bool ipv6_only) {
	return acceptor_->Open(SocketAddress(IPAddress(address), static_cast<u16>(port)), backlog, ipv6_only);
}

}
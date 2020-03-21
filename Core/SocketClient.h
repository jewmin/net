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

#ifndef Net_Core_SocketClient_INCLUDED
#define Net_Core_SocketClient_INCLUDED

#include "Core/SocketWrapperMgr.h"
#include "Reactor/SocketConnector.h"

namespace Net {

class SocketClient : public SocketWrapperMgr {
public:
	SocketClient(const std::string & name, EventReactor * reactor, SocketConnector * connector, i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketClient();

	virtual bool Connect(const std::string & address, i32 port, u32 & id);
	virtual bool Terminate();

protected:
	EventReactor * reactor_;
	SocketConnector * connector_;
	i32 max_out_buffer_size_;
	i32 max_in_buffer_size_;
};

}

#endif
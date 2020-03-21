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

#ifndef Net_Core_SocketConnectionImpl_INCLUDED
#define Net_Core_SocketConnectionImpl_INCLUDED

#include "Core/INotification.h"
#include "Reactor/SocketConnection.h"

namespace Net {

class SocketConnectionImpl : public SocketConnection {
public:
	SocketConnectionImpl(INotification * notification, i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~SocketConnectionImpl();

	virtual void Destroy();
	virtual void OnConnected();
	virtual void OnConnectFailed(int reason);
	virtual void OnDisconnected(bool is_remote);
	virtual void OnNewDataReceived();
	virtual void OnSomeDataSent();
	virtual void OnError(int reason);

private:
	INotification * notification_;
};

}

#endif
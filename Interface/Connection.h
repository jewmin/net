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

#ifndef Net_Interface_Connection_INCLUDED
#define Net_Interface_Connection_INCLUDED

#include "Reactor/SocketConnection.h"

namespace Net {

class ConnectionMgr;
class Connection : public SocketConnection {
public:
	Connection(ConnectionMgr * mgr, i32 max_out_buffer_size, i32 max_in_buffer_size);
	virtual ~Connection();

	ConnectionMgr * GetMgr() const;
	i64 GetConnectionId() const;
	void SetConnectionId(i64 id);
	bool IsRegister2Mgr() const;
	void SetRegister2Mgr(bool is_register2mgr);

protected:
	virtual void OnConnected() override;
	virtual void OnConnectFailed(i32 reason) override;
	virtual void OnDisconnected(bool is_remote) override;
	virtual void OnNewDataReceived() override;
	virtual void OnSomeDataSent() override;
	virtual void OnError(i32 reason) override;

private:
	ConnectionMgr * mgr_;
	i64 connection_id_;
	bool is_register2mgr_;
};

inline ConnectionMgr * Connection::GetMgr() const {
	return mgr_;
}

inline i64 Connection::GetConnectionId() const {
	return connection_id_;
}

inline void Connection::SetConnectionId(i64 id) {
	connection_id_ = id;
}

inline bool Connection::IsRegister2Mgr() const {
	return is_register2mgr_;
}

inline void Connection::SetRegister2Mgr(bool is_register2mgr) {
	is_register2mgr_ = is_register2mgr;
}

}

#endif
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

#ifndef Net_Core_ConnectionMgr_INCLUDED
#define Net_Core_ConnectionMgr_INCLUDED

#include "Common/NetObject.h"
#include "Common/ObjectMgr.h"
#include "Core/INotification.h"
#include "Core/Connection.h"

namespace Net {

class ConnectionMgr : public NetObject {
public:
	void ShutDownAllConnections();
	void ShutDownOneConnection(i64 id);
	virtual void Update();

	std::string GetName() const;
	u32 GetConnectionCount() const;
	Connection * GetConnection(i64 id);
	void SetNotification(INotification * notification);

	void OnConnected(Connection * connection);
	void OnConnectFailed(Connection * connection, i32 reason);
	void OnDisconnected(Connection * connection, bool is_remote);
	void OnNewDataReceived(Connection * connection);
	void OnSomeDataSent(Connection * connection);

protected:
	ConnectionMgr(const std::string & name, u32 object_max_count);
	virtual ~ConnectionMgr();

	virtual i64 Register(Connection * connection);
	virtual void UnRegister(Connection * connection);
	void CleanDeathConnections();

private:
	static void DestroyConnection(Connection * connection, void * ud);
	static void ShutdownConnection(Connection * connection, void * ud);

private:
	std::string name_;
	INotification * notification_;
	ObjectMgr<Connection> * object_mgr_;
	std::list<Connection *> * need_to_delete_list_;
};

inline std::string ConnectionMgr::GetName() const {
	return name_;
}

inline u32 ConnectionMgr::GetConnectionCount() const {
	return object_mgr_->GetObjCount();
}

inline Connection * ConnectionMgr::GetConnection(i64 id) {
	return object_mgr_->GetObj(id);
}

inline void ConnectionMgr::SetNotification(INotification * notification) {
	notification_ = notification;
}

}

#endif
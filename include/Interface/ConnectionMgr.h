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

#ifndef Net_Interface_ConnectionMgr_INCLUDED
#define Net_Interface_ConnectionMgr_INCLUDED

#include "Common/NetObject.h"
#include "Common/ObjectMgr.h"
#include "Interface/INotification.h"
#include "Interface/Connection.h"

namespace Net {

class NET_EXTERN ConnectionMgr : public NetObject {
	friend class Connection;
public:
	virtual ~ConnectionMgr();
	void Update();
	void ShutDownAllConnections();
	void ShutDownOneConnection(i64 id, bool now = false);

	i64 GetMgrId() const;
	void SetMgrId(i64 mgr_id);
	const std::string GetName() const;
	u32 GetConnectionCount() const;
	Connection * GetConnection(i64 id);
	void SetNotification(INotification * notification);

protected:
	ConnectionMgr(const std::string & name);

	void CleanDeathConnections();
	virtual void Register(Connection * connection);
	virtual void UnRegister(Connection * connection);

private:
	static void DestroyConnection(Connection * connection, void * ud);
	static void ShutdownConnection(Connection * connection, void * ud);

private:
	ConnectionMgr(ConnectionMgr &&) = delete;
	ConnectionMgr(const ConnectionMgr &) = delete;
	ConnectionMgr & operator=(ConnectionMgr &&) = delete;
	ConnectionMgr & operator=(const ConnectionMgr &) = delete;

private:
	i64 mgr_id_;
	const std::string name_;
	INotification * notification_;
	ObjectMgr<Connection> * object_mgr_;
	std::list<i64> * need_delete_list_;
};

inline i64 ConnectionMgr::GetMgrId() const {
	return mgr_id_;
}

inline void ConnectionMgr::SetMgrId(i64 mgr_id) {
	mgr_id_ = mgr_id;
}

inline const std::string ConnectionMgr::GetName() const {
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
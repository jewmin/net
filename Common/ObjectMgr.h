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

#ifndef Net_Common_ObjectMgr_INCLUDED
#define Net_Common_ObjectMgr_INCLUDED

#include "Common/NetObject.h"
#include "Common/Allocator.h"
#include "Common/Logger.h"

namespace Net {

template<class OBJECT>
class ObjectMgr : public NetObject {
public:
	ObjectMgr();
	virtual ~ObjectMgr();

	u32 GetObjCount();
	OBJECT * GetObj(i64 id);
	i64 AddNewObj(OBJECT * object);
	i64 AddNewObjById(i64 id, OBJECT * object);
	OBJECT * RemoveObj(i64 id);
	void VisitObj(void(*VisitFunc)(OBJECT * object, void * ud), void * ud);

private:
	ObjectMgr(ObjectMgr &&) = delete;
	ObjectMgr(const ObjectMgr &) = delete;
	ObjectMgr & operator=(ObjectMgr &&) = delete;
	ObjectMgr & operator=(const ObjectMgr &) = delete;

private:
	mutable std::atomic<i64> counter_;
	std::unordered_map<i64, OBJECT *> * objects_;
};

template<class OBJECT>
ObjectMgr<OBJECT>::ObjectMgr() : counter_(0), objects_(new std::unordered_map<i64, OBJECT *>()) {
}

template<class OBJECT>
ObjectMgr<OBJECT>::~ObjectMgr() {
	delete objects_;
}

template<class OBJECT>
u32 ObjectMgr<OBJECT>::GetObjCount() {
	return static_cast<u32>(objects_->size());
}

template<class OBJECT>
OBJECT * ObjectMgr<OBJECT>::GetObj(i64 id) {
	auto got = objects_->find(id);
	if (got == objects_->end()) {
		return nullptr;
	} else {
		return got->second;
	}
}

template<class OBJECT>
i64 ObjectMgr<OBJECT>::AddNewObj(OBJECT * object) {
	if (!object) {
		Log(kCrash, __FILE__, __LINE__, "AddNewObj() object == nullptr");
	}

	i64 id = counter_++;
	if (id >= 0) {
		if (GetObj(id)) {
			Log(kCrash, __FILE__, __LINE__, "AddNewObj() id [", id, "] already exists");
		}
		objects_->insert({id, object});
	}
	return id;
}

template<class OBJECT>
i64 ObjectMgr<OBJECT>::AddNewObjById(i64 id, OBJECT * object) {
	if (id < 0) {
		Log(kCrash, __FILE__, __LINE__, "AddNewObjById() id [", id, "] < 0");
	}
	if (!object) {
		Log(kCrash, __FILE__, __LINE__, "AddNewObjById() object == nullptr");
	}
	if (GetObj(id)) {
		Log(kCrash, __FILE__, __LINE__, "AddNewObjById() id [", id, "] already exists");
	}
	objects_->insert({id, object});
	return id;
}

template<class OBJECT>
OBJECT * ObjectMgr<OBJECT>::RemoveObj(i64 id) {
	if (id < 0) {
		Log(kCrash, __FILE__, __LINE__, "RemoveObj() id [", id, "] < 0");
	}

	OBJECT * object = GetObj(id);
	if (object) {
		objects_->erase(id);
	}
	return object;
}

template<class OBJECT>
void ObjectMgr<OBJECT>::VisitObj(void(*VisitFunc)(OBJECT * object, void * ud), void * ud) {
	for (auto & it : *objects_) {
		VisitFunc(it.second, ud);
	}
}

}

#endif
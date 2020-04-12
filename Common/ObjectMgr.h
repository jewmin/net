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
	ObjectMgr(u32 object_max_count);
	virtual ~ObjectMgr();

	u32 GetObjCount();
	OBJECT * GetObj(i64 id);
	i64 AddNewObj(OBJECT * object);
	OBJECT * RemoveObj(i64 id);
	void VisitObj(void(*VisitFunc)(OBJECT * object, void * ud), void * ud);

protected:
	u8 FFS(u64 x);

private:
	u64 * object_bits_;
	std::unordered_map<i64, OBJECT *> objects_;
	const u32 kObjectBitsMaxSize;

	static const u32 kBitShift = 6;
	static const u32 kBitBytes = sizeof(u64);
	static const u32 kBitCount = kBitBytes << 3;
	static const u32 kBitMask = kBitCount - 1;
	static const u64 kMaxValue = -1;
};

template<class OBJECT>
ObjectMgr<OBJECT>::ObjectMgr(u32 object_max_count) : object_bits_(nullptr), kObjectBitsMaxSize(object_max_count >> kBitShift) {
	if (0 == kObjectBitsMaxSize) {
		Log(kCrash, __FILE__, __LINE__, "ObjectMgr() kObjectBitsMaxSize is zero");
	}
}

template<class OBJECT>
ObjectMgr<OBJECT>::~ObjectMgr() {
	if (object_bits_) {
		jc_free(object_bits_);
	}
}

template<class OBJECT>
u32 ObjectMgr<OBJECT>::GetObjCount() {
	return static_cast<u32>(objects_.size());
}

template<class OBJECT>
OBJECT * ObjectMgr<OBJECT>::GetObj(i64 id) {
	auto got = objects_.find(id);
	if (got == objects_.end()) {
		return nullptr;
	} else {
		return got->second;
	}
}

template<class OBJECT>
inline u8 ObjectMgr<OBJECT>::FFS(u64 x) {
	u8 num = 0;
	if (0 == (x & 0xffffffff)) {
		num += 32;
		x >>= 32;
	}
	if (0 == (x & 0xffff)) {
		num += 16;
		x >>= 16;
	}
	if (0 == (x & 0xff)) {
		num += 8;
		x >>= 8;
	}
	if (0 == (x & 0xf)) {
		num += 4;
		x >>= 4;
	}
	if (0 == (x & 3)) {
		num += 2;
		x >>= 2;
	}
	if (0 == (x & 1)) {
		num += 1;
	}
	return num;
}

template<class OBJECT>
i64 ObjectMgr<OBJECT>::AddNewObj(OBJECT * object) {
	if (!object) {
		Log(kCrash, __FILE__, __LINE__, "AddNewObj() object is nullptr");
	}

	if (!object_bits_) {
		object_bits_ = static_cast<u64 *>(jc_calloc(kObjectBitsMaxSize, kBitBytes));
	}

	i64 id = -1;
	for (u32 i = 0; i < kObjectBitsMaxSize; ++i) {
		if (kMaxValue == object_bits_[i]) {
			continue;
		}
		u8 bit = 0;
		if (0 != object_bits_[i]) {
			bit = FFS(object_bits_[i] ^ kMaxValue);
		}
		object_bits_[i] |= static_cast<u64>(1) << bit;
		id = (i << kBitShift) + bit;
		break;
	}

	if (id >= 0) {
		if (GetObj(id)) {
			Log(kCrash, __FILE__, __LINE__, "AddNewObj() id already exists", id);
		}
		objects_.insert({id, object});
	}
	return id;
}

template<class OBJECT>
OBJECT * ObjectMgr<OBJECT>::RemoveObj(i64 id) {
	if (id < 0) {
		Log(kCrash, __FILE__, __LINE__, "RemoveObj() id must be >= 0", id);
	}

	OBJECT * object = GetObj(id);
	if (object) {
		objects_.erase(id);
		u32 idx = static_cast<u32>(id >> kBitShift);
		u64 mask = static_cast<u64>(1) << (id & kBitMask);
		object_bits_[idx] &= ~mask;
	}
	return object;
}

template<class OBJECT>
void ObjectMgr<OBJECT>::VisitObj(void(*VisitFunc)(OBJECT * object, void * ud), void * ud) {
	for (auto & it : objects_) {
		VisitFunc(it.second, ud);
	}
}

}

#endif
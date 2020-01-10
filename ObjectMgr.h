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

#ifndef Foundation_ObjectMgr_INCLUDED
#define Foundation_ObjectMgr_INCLUDED

#include "Net.h"

namespace Foundation {
	template<class OBJECT>
	class ObjectMgr {
		struct CHUNK {
			u32 object_count;
			OBJECT * * objects;
		};

	public:
		ObjectMgr(u32 objectCount, u32 chunkCount);
		~ObjectMgr();

		u32 GetObjCount();
		OBJECT * GetObj(u32 id);
		u32 AddNewObj(OBJECT * object);
		u32 AddNewObj(u32 id, OBJECT * object);
		OBJECT * RemoveObj(u32 id);
		bool DeleteObj(u32 id);
		void DeleteAllObjs();
		void EnumEachObj(void(*EachFunc)(void * object, void * ud), void * ud);

	protected:
		CHUNK * AllocOneChunk();

	private:
		u32 chunk_mod_;
		u32 object_mod_;
		u32 object_count_per_chunk_;
		u32 init_chunk_count_;
		u32 next_chunk_index_;
		u32 next_object_index_;
		u32 next_object_id_;
		std::vector<CHUNK *> active_chunk_;
		std::list<CHUNK *> free_chunk_;
	};
}

template<class OBJECT>
Foundation::ObjectMgr<OBJECT>::ObjectMgr(u32 objectCount, u32 chunkCount)
	: init_chunk_count_(chunkCount), next_chunk_index_(0), next_object_index_(1), next_object_id_(1) {
	if (objectCount > 8) {
		if (objectCount < 4096) {
			chunk_mod_ = 0;
			object_count_per_chunk_ = 1;
			--objectCount;
			while (objectCount > 0) {
				objectCount >>= 1;
				object_count_per_chunk_ <<= 1;
				++chunk_mod_;
			}
		} else {
			object_count_per_chunk_ = 4096;
			chunk_mod_ = 12;
		}
	} else {
		object_count_per_chunk_ = 8;
		chunk_mod_ = 3;
	}
	object_mod_ = object_count_per_chunk_ - 1;
	active_chunk_.reserve(init_chunk_count_);
}

template<class OBJECT>
Foundation::ObjectMgr<OBJECT>::~ObjectMgr() {
	DeleteAllObjs();
}

template<class OBJECT>
typename Foundation::ObjectMgr<OBJECT>::CHUNK * Foundation::ObjectMgr<OBJECT>::AllocOneChunk() {
	CHUNK * chunk = nullptr;
	if (!free_chunk_.empty()) {
		chunk = free_chunk_.back();
		free_chunk_.pop_back();
	} else {
		chunk = new CHUNK();
		chunk->objects = static_cast<OBJECT * *>(malloc(sizeof(OBJECT *) * object_count_per_chunk_));
		chunk->object_count = 0;
		for (u32 i = 0; i < object_count_per_chunk_; ++i) {
			*(chunk->objects + i) = nullptr;
		}
	}
	return chunk;
}

template<class OBJECT>
u32 Foundation::ObjectMgr<OBJECT>::GetObjCount() {
	u32 count = 0;
	for (auto & it : active_chunk_) {
		CHUNK * chunk = it;
		if (chunk && chunk->object_count > 0) {
			count += chunk->object_count;
		}
	}
	return count;
}

template<class OBJECT>
OBJECT * Foundation::ObjectMgr<OBJECT>::GetObj(u32 id) {
	if (0 == id || id >= next_object_id_) {
		return nullptr;
	}
	u32 chunk_index = id >> chunk_mod_;
	u32 object_index = id & object_mod_;
	CHUNK * chunk = active_chunk_[chunk_index];
	if (chunk) {
		return *(chunk->objects + object_index);
	}
	return nullptr;
}

template<class OBJECT>
u32 Foundation::ObjectMgr<OBJECT>::AddNewObj(OBJECT * object) {
	if (!object) {
		throw std::invalid_argument("object is nullptr");
	}
	CHUNK * chunk = nullptr;
	if (next_chunk_index_ == active_chunk_.size()) {
		chunk = AllocOneChunk();
		active_chunk_.push_back(chunk);
	} else {
		chunk = active_chunk_.back();
	}
	*(chunk->objects + next_object_index_) = object;
	++chunk->object_count;
	if (++next_object_index_ == object_count_per_chunk_) {
		next_object_index_ = 0;
		++next_chunk_index_;
	}
	return next_object_id_++;
}

template<class OBJECT>
u32 Foundation::ObjectMgr<OBJECT>::AddNewObj(u32 id, OBJECT * object) {
	if (0 == id) {
		throw std::out_of_range("0 < id");
	} else if (!object) {
		throw std::invalid_argument("object is nullptr");
	}
	if (id >= next_object_id_) {
		next_object_id_ = id + 1;
	}
	u32 chunk_index = id >> chunk_mod_;
	u32 object_index = id & object_mod_;
	if (chunk_index >= active_chunk_.size()) {
		for (u32 i = static_cast<u32>(active_chunk_.size()); chunk_index >= i; ++i) {
			active_chunk_.push_back(AllocOneChunk());
		}
	}
	if (next_chunk_index_ < chunk_index) {
		next_chunk_index_ = chunk_index;
	}
	if (next_chunk_index_ == chunk_index) {
		if (object_index + 1 == object_count_per_chunk_) {
			next_object_index_ = 0;
			++next_chunk_index_;
		} else if (object_index >= next_object_index_) {
			next_object_index_ = object_index + 1;
		}
	}
	CHUNK * chunk = active_chunk_[chunk_index];
	if (!chunk) {
		chunk = AllocOneChunk();
		active_chunk_[chunk_index] = chunk;
	}
	if (*(chunk->objects + object_index)) {
		throw std::invalid_argument("id position has object");
	}
	*(chunk->objects + object_index) = object;
	++chunk->object_count;
	return id;
}

template<class OBJECT>
OBJECT * Foundation::ObjectMgr<OBJECT>::RemoveObj(u32 id) {
	if (0 == id || id >= next_object_id_) {
		throw std::out_of_range("0 < id < next_object_id_");
	}
	u32 chunk_index = id >> chunk_mod_;
	u32 object_index = id & object_mod_;
	CHUNK * chunk = active_chunk_[chunk_index];
	if (!chunk) {
		return nullptr;
	}
	OBJECT * object = *(chunk->objects + object_index);
	if (object) {
		*(chunk->objects + object_index) = nullptr;
		if (--chunk->object_count == 0) {
			if (next_chunk_index_ != chunk_index) {
				free_chunk_.push_back(chunk);
				active_chunk_[chunk_index] = nullptr;
			}
		}
	}
	return object;
}

template<class OBJECT>
bool Foundation::ObjectMgr<OBJECT>::DeleteObj(u32 id) {
	OBJECT * object = RemoveObj(id);
	if (!object) {
		return false;
	} else {
		delete object;
		return true;
	}
}

template<class OBJECT>
void Foundation::ObjectMgr<OBJECT>::DeleteAllObjs() {
	for (auto & it : active_chunk_) {
		CHUNK * chunk = it;
		if (chunk) {
			if (chunk->object_count > 0) {
				for (u32 i = 0; i < object_count_per_chunk_; ++i) {
					OBJECT * object = *(chunk->objects + i);
					if (object) {
						delete object;
					}
				}
			}
			if (chunk->objects) {
				free(chunk->objects);
			}
			delete chunk;
		}
	}
	active_chunk_.clear();
	active_chunk_.reserve(init_chunk_count_);
	next_chunk_index_ = 0;
	next_object_index_ = 1;
	next_object_id_ = 1;
	while (!free_chunk_.empty()) {
		CHUNK * chunk = free_chunk_.back();
		if (chunk->objects) {
			free(chunk->objects);
		}
		delete chunk;
		free_chunk_.pop_back();
	}
}

template<class OBJECT>
void Foundation::ObjectMgr<OBJECT>::EnumEachObj(void(*EachFunc)(void * object, void * ud), void * ud) {
	for (auto & it : active_chunk_) {
		CHUNK * chunk = it;
		if (chunk && chunk->object_count > 0) {
			for (u32 i = 0; i < object_count_per_chunk_; ++i) {
				OBJECT * object = *(chunk->objects + i);
				if (object) {
					EachFunc(static_cast<void *>(object), ud);
				}
			}
		}
	}
}

#endif
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

#ifndef Net_Allocator_INCLUDED
#define Net_Allocator_INCLUDED

#include "Net.h"

namespace Net {

class Allocator {
	struct Obj { struct Obj * next; };
	struct Chunk { struct Chunk * next; i32 size; };

public:
	Allocator();
	virtual ~Allocator();
	Allocator(const Allocator &) = delete;
	Allocator & operator=(const Allocator &) = delete;

	void * Allocate(i32 size);
	void DeAllocate(void * ptr, i32 size);
	void Stat(int & alloc, int & used);
	void SetMutex(uv_mutex_t * mutex);
	static Allocator * Get();

protected:
	void * Refill(i32 size);
	void * AllocateChunk(i32 size, i32 & num);

private:
	i32 SmallSizeClass(i32 size);
	i32 LargeSizeClass(i32 size);
	i32 RoundUpSize(i32 size);
	struct Obj * * GetFreeList(i32 size);
	void AppendToFreeList(void * ptr, i32 size);
	void Lock();
	void Unlock();

protected:
	static const i32 SmallAlign = 8;
	static const i32 SmallMaxBytes = 256;
	static const i32 SmallFreeListSize = SmallMaxBytes / SmallAlign;
	static const i32 LargeAlign = 64;
	static const i32 LargeMaxBytes = 2048;
	static const i32 LargeFreeListSize = LargeMaxBytes / LargeAlign;
	static const i32 LargeOffset = LargeFreeListSize - SmallMaxBytes / LargeAlign;
	static const i32 TotalFreeListSize = SmallFreeListSize + LargeFreeListSize;

	struct Obj * free_list_[TotalFreeListSize];
	struct Chunk * chunk_list_;
	uv_mutex_t * mutex_;
};

inline i32 Allocator::SmallSizeClass(i32 size) {
	return (size + SmallAlign - 1) / SmallAlign;
}

inline i32 Allocator::LargeSizeClass(i32 size) {
	return (size + LargeAlign - 1) / LargeAlign + LargeOffset;
}

inline i32 Allocator::RoundUpSize(i32 size) {
	if (size <= SmallMaxBytes) {
		return (size + SmallAlign - 1) & ~(SmallAlign - 1);
	} else {
		return (size + LargeAlign - 1) & ~(LargeAlign - 1);
	}
}

inline struct Allocator::Obj * * Allocator::GetFreeList(i32 size) {
	if (size <= SmallMaxBytes) {
		return free_list_ + SmallSizeClass(size);
	} else {
		return free_list_ + LargeSizeClass(size);
	}
}

inline void Allocator::SetMutex(uv_mutex_t * mutex) {
	mutex_ = mutex;
}

inline void Allocator::Lock() {
	if (mutex_) {
		uv_mutex_lock(mutex_);
	}
}

inline void Allocator::Unlock() {
	if (mutex_) {
		uv_mutex_unlock(mutex_);
	}
}

}

#endif
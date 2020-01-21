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

#include "Allocator.h"

namespace Net {

typedef struct {
	jc_malloc_func local_malloc;
	jc_realloc_func local_realloc;
	jc_calloc_func local_calloc;
	jc_free_func local_free;
} jc_allocator_t;

static jc_allocator_t jc_allocator = {
	std::malloc,
	std::realloc,
	std::calloc,
	std::free,
};


Allocator::Allocator() {
	for (int i = 0; i < TotalFreeListSize; ++i) {
		free_list_[i] = nullptr;
	}
}

Allocator::~Allocator() {
}

void * Allocator::Allocate(i32 size) {
	if (size > LargeMaxBytes) {
		return jc_allocator.local_malloc(size);
	} else {
		struct Obj * * free_list = GetFreeList(size);
		struct Obj * result = *free_list;
		if (nullptr == result) {
			result = static_cast<Obj *>(Refill(RoundUpSize(size)));
		} else {
			*free_list = result->next;
		}
		return result;
	}
}

void Allocator::DeAllocate(void * ptr, i32 size) {
	if (nullptr == ptr) {
		return;
	}

	if (size > LargeMaxBytes) {
		jc_allocator.local_free(ptr);
	} else {
		struct Obj * * free_list = GetFreeList(size);
		struct Obj * result = static_cast<struct Obj *>(ptr);
		result->next = *free_list;
		*free_list = result;
	}
}

void * Allocator::Refill(i32 size) {
	i32 num = 20;
	void * chunk = AllocateChunk(size, num);
	if (nullptr == chunk || 1 == num) {
		return chunk;
	}

	struct Obj * * free_list = GetFreeList(size);
	struct Obj * result = static_cast<struct Obj *>(chunk);
	struct Obj * current = nullptr;
	struct Obj * next = nullptr;
	*free_list = next = reinterpret_cast<struct Obj *>(static_cast<i8 *>(chunk) + size);
	for (int i = 1; ; ++i) {
		current = next;
		next = reinterpret_cast<struct Obj *>(reinterpret_cast<i8 *>(next) + size);
		if (num - 1 == i) {
			current->next = nullptr;
			break;
		} else {
			current->next = next;
		}
	}
	return result;
}

void * Allocator::AllocateChunk(i32 size, i32 & num) {
	i32 total_size = size * num;
	void * result = jc_allocator.local_malloc(total_size);
	if (nullptr != result) {
		return result;
	}

	for (int i = size; i <= SmallMaxBytes; i += SmallAlign) {
		struct Obj * * free_list = GetFreeList(i);
		struct Obj * ptr = *free_list;
		if (nullptr != ptr) {
			*free_list = ptr->next;
			num = i / size;
			return ptr;
		}
	}

	for (int i = SmallMaxBytes + LargeAlign; i <= LargeMaxBytes; i += LargeAlign) {
		struct Obj * * free_list = GetFreeList(i);
		struct Obj * ptr = *free_list;
		if (nullptr != ptr) {
			*free_list = ptr->next;
			num = i / size;
			return ptr;
		}
	}

	return nullptr;
}

}
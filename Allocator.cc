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

int jc_replace_allocator(jc_malloc_func malloc_func, jc_realloc_func realloc_func, jc_calloc_func calloc_func, jc_free_func free_func) {
	if (nullptr == malloc_func || nullptr == realloc_func || nullptr == calloc_func || nullptr == free_func) {
		return -1;
	}

	jc_allocator.local_malloc = malloc_func;
	jc_allocator.local_realloc = realloc_func;
	jc_allocator.local_calloc = calloc_func;
	jc_allocator.local_free = free_func;
	return 0;
}

namespace Net {

Allocator::Allocator() {
	for (int i = 0; i < TotalFreeListSize; ++i) {
		free_list_[i] = nullptr;
	}
	chunk_list_ = nullptr;
}

Allocator::~Allocator() {
	struct Chunk * next = nullptr;
	while (chunk_list_) {
		next = chunk_list_->next;
		jc_allocator.local_free(chunk_list_);
		chunk_list_ = next;
	}
}

void * Allocator::Allocate(i32 size) {
	if (size > LargeMaxBytes) {
		return jc_allocator.local_malloc(size);
	} else {
		struct Obj * * free_list = GetFreeList(size);
		struct Obj * result = *free_list;
		if (result) {
			*free_list = result->next;
		} else {
			result = static_cast<Obj *>(Refill(RoundUpSize(size)));
		}
		return result;
	}
}

void Allocator::DeAllocate(void * ptr, i32 size) {
	if (!ptr || 0 == size) {
		return;
	}

	if (size > LargeMaxBytes) {
		jc_allocator.local_free(ptr);
	} else {
		AppendToFreeList(ptr, size);
	}
}

void * Allocator::Refill(i32 size) {
	i32 num = 20;
	void * chunk = AllocateChunk(size, num);
	if (!chunk || 1 == num) {
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
	struct Chunk * result = static_cast<struct Chunk *>(jc_allocator.local_malloc(sizeof(struct Chunk) + total_size));
	if (result) {
		result->size = total_size;
		result->next = chunk_list_;
		chunk_list_ = result;
		return result + 1;
	}

	for (int i = size; i <= SmallMaxBytes; i += SmallAlign) {
		struct Obj * * free_list = GetFreeList(i);
		struct Obj * ptr = *free_list;
		if (ptr) {
			*free_list = ptr->next;
			num = i / size;
			int left = i - size * num;
			if (left > 0) {
				AppendToFreeList(reinterpret_cast<i8 *>(ptr) + size * num, left);
			}
			return ptr;
		}
	}

	for (int i = SmallMaxBytes + LargeAlign; i <= LargeMaxBytes; i += LargeAlign) {
		struct Obj * * free_list = GetFreeList(i);
		struct Obj * ptr = *free_list;
		if (ptr) {
			*free_list = ptr->next;
			num = i / size;
			int left = i - size * num;
			if (left > 0) {
				AppendToFreeList(reinterpret_cast<i8 *>(ptr) + size * num, left);
			}
			return ptr;
		}
	}

	return nullptr;
}

void Allocator::AppendToFreeList(void * ptr, i32 size) {
	struct Obj * * free_list = GetFreeList(size);
	struct Obj * result = static_cast<struct Obj *>(ptr);
	result->next = *free_list;
	*free_list = result;
}

void Allocator::Stat(int & alloc, int & used) {
	used = alloc = 0;

	struct Chunk * next_chunk = chunk_list_;
	while (next_chunk) {
		alloc += next_chunk->size;
		next_chunk = next_chunk->next;
	}

	int i, j, unused = 0;
	struct Obj * next_obj = nullptr;
	for (i = 1, j = SmallAlign; j <= SmallMaxBytes; ++i, j += SmallAlign) {
		next_obj = free_list_[i];
		while (next_obj) {
			unused += j;
			next_obj = next_obj->next;
		}
	}
	for (j = SmallMaxBytes + LargeAlign; j <= LargeMaxBytes; ++i, j += LargeAlign) {
		next_obj = free_list_[i];
		while (next_obj) {
			unused += j;
			next_obj = next_obj->next;
		}
	}

	used = alloc - unused;
}

}
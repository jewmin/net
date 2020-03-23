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

#include "Common/Allocator.h"

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

bool jc_replace_allocator(jc_malloc_func malloc_func, jc_realloc_func realloc_func, jc_calloc_func calloc_func, jc_free_func free_func) {
	if (nullptr == malloc_func || nullptr == realloc_func || nullptr == calloc_func || nullptr == free_func) {
		return false;
	}

	jc_allocator.local_malloc = malloc_func;
	jc_allocator.local_realloc = realloc_func;
	jc_allocator.local_calloc = calloc_func;
	jc_allocator.local_free = free_func;
	return true;
}

void * jc_malloc(size_t size) {
	return jc_allocator.local_malloc(size);
}

void * jc_realloc(void * ptr, size_t size) {
	return jc_allocator.local_realloc(ptr, size);
}

void * jc_calloc(size_t count, size_t size) {
	return jc_allocator.local_calloc(count, size);
}

void jc_free(void * ptr) {
	jc_allocator.local_free(ptr);
}
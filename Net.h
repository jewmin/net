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

#ifndef Net_INCLUDED
#define Net_INCLUDED

// libuv库头文件
#include "uv.h"

// stl标准库头文件
#include <new>
#include <set>
#include <map>
#include <list>
#include <ctime>
#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>

// C头文件
#include <assert.h>
#ifdef _WIN32
#	include <io.h>
#	include <process.h>
#else
#	include <unistd.h>
#endif

// 标准描述符
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

// 定义标准类型，内部统一使用
typedef char				i8;
typedef short				i16;
typedef int					i32;
typedef long long			i64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

// 定义内存分配原子函数
typedef void * (*jc_malloc_func)(size_t size);
typedef void * (*jc_realloc_func)(void * ptr, size_t size);
typedef void * (*jc_calloc_func)(size_t count, size_t size);
typedef void (*jc_free_func)(void * ptr);

// 定义日志打印原子函数
typedef void (*log_message_writer)(const i8 * msg, i32 length);

// 定义日志崩溃处理函数
typedef void (*log_abort)();

// 动态库开放函数，采用C语言函数
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	/* Windows - set up dll import/export decorators. */
#	if defined(BUILDING_NET_SHARED)
		/* Building shared library. */
#		define NET_EXTERN __declspec(dllexport)
#	elif defined(USING_NET_SHARED)
		/* Using shared library. */
#		define NET_EXTERN __declspec(dllimport)
#	else
		/* Building static library. */
#		define NET_EXTERN /* nothing */
#	endif
#elif __GNUC__ >= 4
#	define NET_EXTERN __attribute__((visibility("default")))
#else
#	define NET_EXTERN /* nothing */
#endif

NET_EXTERN bool jc_replace_allocator(jc_malloc_func malloc_func, jc_realloc_func realloc_func, jc_calloc_func calloc_func, jc_free_func free_func);
NET_EXTERN bool jc_replace_logger(log_message_writer log_func);
NET_EXTERN bool jc_replace_abort(log_abort abort_func);

#ifdef __cplusplus
}
#endif

#endif
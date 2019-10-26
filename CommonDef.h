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

#ifndef CommonDef_INCLUDED
#define CommonDef_INCLUDED

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
#include <unordered_map>
#include <assert.h>

typedef char				i8;
typedef short				i16;
typedef int					i32;
typedef long long			i64;

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;

template<class S>
S Trim(const S & str) {
	int first = 0;
	int last = int(str.size()) - 1;

	while (first <= last && 0x20 == str[first]) ++first;
	while (last >= first && 0x20 == str[last]) --last;

	return S(str, first, last - first + 1);
}

#endif
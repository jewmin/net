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

#ifndef Foundation_PacketReader_INCLUDED
#define Foundation_PacketReader_INCLUDED

#include "CommonDef.h"

namespace Foundation {
	class PacketReader {
	public:
		PacketReader() : mem_ptr_(nullptr), end_ptr_(nullptr), offset_(nullptr), data_end_(nullptr) {
		}

		PacketReader(char * buff, int size) : mem_ptr_(buff), end_ptr_(buff + size), offset_(buff), data_end_(buff + size) {
		}

		// 获取内存空间的大小
		inline int GetLength() {
			return end_ptr_ - mem_ptr_;
		}

		// 获取数据包的当前有效数据长度
		inline int GetDataLength() {
			return data_end_ - mem_ptr_;
		}

		// 获取自当前读写指针开始可以读取的剩余字节数
		inline int GetReadableLength() {
			return data_end_ - offset_;
		}

		// 获取数据包当前读写位置字节偏移量
		inline int GetPosition() {
			return offset_ - mem_ptr_;
		}

		// 获取数据包数据内存指针
		inline char * GetMemoryPtr() {
			return mem_ptr_;
		}

		// 获取数据包数据内存的当前偏移指针
		inline char * GetOffsetPtr() {
			return offset_;
		}

		// 获取指定偏移量的指针
		inline char * GetPositionPtr(int pos) {
			return mem_ptr_ + pos;
		}

		// 设置数据包当前读写位置字节偏移量，如果新的偏移量比当前内存块长度要大，则限制新的偏移位置不超过当前内存块长度
		virtual int SetPosition(int new_pos) {
			int mem_size = GetLength();
			if (new_pos > mem_size) {
				new_pos = mem_size;
			}
			offset_ = mem_ptr_ + new_pos;
			return new_pos;
		}

		// 基于当前读写指针偏移位置调整偏移量
		virtual int AdjustOffset(int adjust_offset) {
			offset_ += adjust_offset;
			if (offset_ < mem_ptr_) {
				offset_ = mem_ptr_;
			} else if (offset_ > end_ptr_) {
				offset_ = end_ptr_;
			}
			return offset_ - mem_ptr_;
		}

		// 读取原子数据
		template<typename T>
		inline T ReadAtom() {
			T value;
			int avaliable = GetReadableLength();
			if (avaliable >= sizeof(T)) {
				value = *(reinterpret_cast<T *>(offset_));
				offset_ += sizeof(T);
			} else {
				memset(&value, 0, sizeof(value));
				offset_ += avaliable;
			}
			return value;
		}

		// 读取二进制数据
		inline int ReadBinary(char * buf, int size) {
			int avaliable = GetReadableLength();
			if (avaliable < size) {
				memset(buf, 0, size);
				size = avaliable;
			}
			memcpy(buf, offset_, size);
			offset_ += size;
			return size;
		}

		// 读取字符串数据，字符串的数据格式为：[2字节字符长度数据][字符串字节数据，字符串长度在65536以内][字符串终止字符0]
		inline int ReadString(char * str, int len) {
			return RawReadStringLength<u16>(str, len);
		}

		inline const char * ReadString() {
			return RawReadStringPtr<u16>();
		}

		template<typename T>
		inline PacketReader & operator >> (T & value) {
			if (sizeof(value) <= sizeof(int)) {
				value = ReadAtom<T>();
			} else {
				ReadBinary(reinterpret_cast<char *>(&value), sizeof(value));
			}
			return *this;
		}

		inline PacketReader & operator >> (const char * & str) {
			str = RawReadStringPtr<u16>();
			return *this;
		}

	protected:
		// 读取字符串到缓冲区
		template<typename LENGTH>
		int RawReadStringLength(char * str, int len) {
			assert(len > 0);
			int avaliable = GetReadableLength(), read_len = 0, str_len = 0;
			if (avaliable >= sizeof(LENGTH)) {
				read_len = str_len = *(reinterpret_cast<LENGTH *>(offset_));
				// 跳过字符串长度数据
				offset_ += sizeof(LENGTH);
				avaliable -= sizeof(LENGTH);
				// 计算实际读取长度，避免越界
				if (read_len > len) {
					read_len = len;
				}
				if (read_len > avaliable) {
					read_len = avaliable;
				}
				if (read_len > 0) {
					ReadBinary(reinterpret_cast<char *>(str), read_len);
				}
				// 如果读取的字符串长度少于数据中指定的字符串长度，则需要跳过没有读取的部分，以便接下来可以正确的从数据包中读取后续的数据
				if (str_len > read_len) {
					offset_ += (str_len - read_len);
				}
				// 跳过终止字符0
				++offset_;
				// offset不能超过data_end的范围
				if (offset_ > data_end_) {
					offset_ = data_end_;
				}
			}
			// 添加终止字符0
			if (read_len >= len) {
				read_len = len - 1;
			}
			str[read_len] = 0;
			// 返回字符串长度
			return str_len;
		}

		// 读取字符串数据
		template<typename LENGTH>
		const char * RawReadStringPtr() {
			int avaliable = GetReadableLength();
			if (avaliable >= sizeof(LENGTH) + 1) {
				int str_len = *(reinterpret_cast<LENGTH *>(offset_));
				// 如果数据包中有足够的字符串数据空间
				if (avaliable >= str_len + sizeof(LENGTH) + 1) {
					const char * str = reinterpret_cast<char *>(offset_ + sizeof(LENGTH));
					offset_ += str_len + sizeof(LENGTH) + 1;
					return str;
				}
			}
			return nullptr;
		}

	private:
		PacketReader(const PacketReader &) = delete;
		PacketReader & operator=(const PacketReader &) = delete;

	protected:
		char * mem_ptr_;
		char * end_ptr_;
		char * offset_;
		char * data_end_;
	};
}

#endif
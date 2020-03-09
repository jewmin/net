#include "gtest/gtest.h"
#include "Net.h"
#include "Common/Allocator.h"
#include "Common/NetObject.h"

void * AllocatorTestSuite_Malloc(size_t size) {
	return nullptr;
}

i32 AllocatorTestSuite_RoundUpSize(i32 size) {
	if (size <= 256) {
		return (size + 7) & ~(7);
	}
	else {
		return (size + 63) & ~(63);
	}
}

TEST(AllocatorTestSuite, alloc) {
	struct object {
		void * ptr;
		int size;
	};
	Net::Allocator allocator;
	std::vector<struct object> all;
	for (int i = 5; i < 2500; i += 5) {
		struct object a = { allocator.Allocate(i), i };
		all.push_back(a);
	}
	int alloc, used, all_used = 0, tmp_used = 0;
	allocator.Stat(alloc, used);
	for (std::vector<struct object>::iterator it = all.begin(); it != all.end(); it++) {
		tmp_used = AllocatorTestSuite_RoundUpSize(it->size);
		if (tmp_used <= 2048) {
			all_used += tmp_used;
		}
	}
	EXPECT_EQ(all_used, used);
	printf("alloc = %d, used = %d all_used = %d\n", alloc, used, all_used);
	for (std::vector<struct object>::iterator it = all.begin(); it != all.end(); it++) {
		allocator.DeAllocate(it->ptr, it->size);
	}
}

TEST(AllocatorTestSuite, object) {
	class TestObject : public Net::NetObject {
	public:
		TestObject(int a) : a_(a) { printf("TestObject construct\n"); }
		virtual ~TestObject() { printf("TestObject destruct\n"); }
		int a_;
		bool b_;
		long c_;
		double d_;
		float e_;
	};

	char buf[64];

	TestObject a(1);
	TestObject * b = new TestObject(2);
	TestObject * c = new(buf)TestObject(3);
	delete b;
}

TEST(AllocatorTestSuite, error) {
	struct object {
		void * ptr;
		int size;
	};
	Net::Allocator allocator;
	std::vector<struct object> all;
	for (int i = 0; i < 20; i++) {
		struct object a = { allocator.Allocate(50), 50 };
		all.push_back(a);
	}

	struct object a = { allocator.Allocate(100), 100 };
	all.push_back(a);
	struct object c = { allocator.Allocate(2000), 2000 };
	all.push_back(c);
	jc_replace_allocator(AllocatorTestSuite_Malloc, realloc, calloc, free);
	struct object b = { allocator.Allocate(50), 50 };
	all.push_back(b);
	struct object d = { allocator.Allocate(256), 256 };
	all.push_back(d);
	struct object e = { allocator.Allocate(300), 300 };
	all.push_back(e);
	int alloc, used, all_used = 0;
	allocator.Stat(alloc, used);
	for (std::vector<struct object>::iterator it = all.begin(); it != all.end(); it++) {
		all_used += AllocatorTestSuite_RoundUpSize(it->size);
	}
	EXPECT_EQ(all_used, used);
	printf("alloc = %d, used = %d all_used = %d\n", alloc, used, all_used);
	for (std::vector<struct object>::iterator it = all.begin(); it != all.end(); it++) {
		allocator.DeAllocate(it->ptr, it->size);
	}
	jc_replace_allocator(malloc, realloc, calloc, free);
}

TEST(AllocatorTestSuite, null) {
	struct object {
		void * ptr;
		int size;
	};
	Net::Allocator allocator;
	std::vector<struct object> all;
	allocator.DeAllocate(nullptr, 100);
	allocator.DeAllocate(reinterpret_cast<void *>(100), 0);
	jc_replace_allocator(AllocatorTestSuite_Malloc, realloc, calloc, free);
	struct object a = { allocator.Allocate(100), 100 };
	all.push_back(a);
	EXPECT_TRUE(a.ptr == nullptr);
	for (std::vector<struct object>::iterator it = all.begin(); it != all.end(); it++) {
		allocator.DeAllocate(it->ptr, it->size);
	}
	jc_replace_allocator(malloc, realloc, calloc, free);
}
#include "gtest/gtest.h"
#include "Common/Allocator.h"

TEST(AllocatorTestSuite, replace) {
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, nullptr, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, std::realloc, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, nullptr, std::calloc, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(nullptr, nullptr, nullptr, std::free));
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, std::realloc, nullptr, nullptr));
	EXPECT_EQ(false, jc_replace_allocator(std::malloc, std::realloc, std::calloc, nullptr));
	EXPECT_EQ(true, jc_replace_allocator(std::malloc, std::realloc, std::calloc, std::free));
}

TEST(AllocatorTestSuite, alloc) {
	int * p = (int *)jc_malloc(sizeof(int));
	*p = 999;
	EXPECT_EQ(*p, 999);

	p = (int *)jc_realloc(p, sizeof(int) << 1);
	*(p + 1) = 88;
	EXPECT_EQ(*p, 999);
	EXPECT_EQ(*(p + 1), 88);

	jc_free(p);

	p = (int *)jc_calloc(3, sizeof(int));
	EXPECT_EQ(*p, 0);
	EXPECT_EQ(*(p + 1), 0);
	EXPECT_EQ(*(p + 2), 0);

	jc_free(p);
}
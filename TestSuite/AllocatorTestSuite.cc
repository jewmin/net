#include "gtest/gtest.h"
#include "Common/Allocator.h"

TEST(AllocatorTestSuite, replace) {
	jc_malloc_func test_malloc[2] = { nullptr, std::malloc };
	jc_realloc_func test_realloc[2] = { nullptr, std::realloc };
	jc_calloc_func test_calloc[2] = { nullptr, std::calloc };
	jc_free_func test_free[2] = { nullptr, std::free };
	for (int i = 0; i < 16; ++i) {
		EXPECT_EQ(i == 15, jc_replace_allocator(test_malloc[i & 1], test_realloc[(i >> 1) & 1], test_calloc[(i >> 2) & 1], test_free[(i >> 3) & 1]));
	}
}

TEST(AllocatorTestSuite, alloc) {
	int * p = (int *)jc_malloc(sizeof(int));
	EXPECT_TRUE(p != nullptr);
	*p = 999;
	EXPECT_EQ(*p, 999);

	p = (int *)jc_realloc(p, sizeof(int) << 1);
	EXPECT_TRUE(p != nullptr);
	*(p + 1) = 88;
	EXPECT_EQ(*p, 999);
	EXPECT_EQ(*(p + 1), 88);

	jc_free(p);

	p = (int *)jc_calloc(3, sizeof(int));
	EXPECT_TRUE(p != nullptr);
	EXPECT_EQ(*p, 0);
	EXPECT_EQ(*(p + 1), 0);
	EXPECT_EQ(*(p + 2), 0);

	jc_free(p);
}
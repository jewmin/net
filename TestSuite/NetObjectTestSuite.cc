#include "gtest/gtest.h"
#include "Common/NetObject.h"
#include "Common/Allocator.h"

class MockNetObject : public Net::NetObject {
public:
	MockNetObject() : a(1), b(1.5), c(2), d(2.5) {}
	virtual ~MockNetObject() {}
	int a;
	float b;
	long c;
	double d;
};

TEST(NetObjectTestSuite, alloc) {
	MockNetObject object;
	EXPECT_EQ(object.a, 1);
	EXPECT_FLOAT_EQ(object.b, 1.5);
	EXPECT_EQ(object.c, 2);
	EXPECT_DOUBLE_EQ(object.d, 2.5);

	MockNetObject * object1 = new MockNetObject();
	EXPECT_TRUE(object1 != nullptr);
	EXPECT_EQ(object1->a, 1);
	EXPECT_FLOAT_EQ(object1->b, 1.5);
	EXPECT_EQ(object1->c, 2);
	EXPECT_DOUBLE_EQ(object1->d, 2.5);
	delete object1;

	char buf[sizeof(MockNetObject)];
	MockNetObject * object2 = reinterpret_cast<MockNetObject *>(const_cast<char *>(buf));
	new(object2)MockNetObject();
	EXPECT_EQ(object2->a, 1);
	EXPECT_FLOAT_EQ(object2->b, 1.5);
	EXPECT_EQ(object2->c, 2);
	EXPECT_DOUBLE_EQ(object2->d, 2.5);
	object2->~MockNetObject();
}
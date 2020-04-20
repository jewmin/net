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
	MockNetObject * o = new MockNetObject();
	delete o;

	MockNetObject * o1 = static_cast<MockNetObject *>(jc_malloc(sizeof(*o1)));
	new(o1)MockNetObject();
	o1->~MockNetObject();
	jc_free(o1);
}
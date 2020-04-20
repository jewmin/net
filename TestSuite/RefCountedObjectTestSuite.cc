#include "gtest/gtest.h"
#include "Common/RefCountedObject.h"
#include "Common/Allocator.h"

class MockRefCountedObject : public Net::RefCountedObject {
public:
	MockRefCountedObject() : a(1) {}
	virtual ~MockRefCountedObject() {}
	int a;
};

TEST(RefCountedObjectTestSuite, counter) {
	Net::RefCounter c;
	EXPECT_EQ((int)c, 1);
	EXPECT_EQ((int)c++, 1);
	EXPECT_EQ((int)++c, 3);
	EXPECT_EQ((int)--c, 2);
	EXPECT_EQ((int)c--, 2);
	EXPECT_EQ((int)c, 1);
}

TEST(RefCountedObjectTestSuite, ref) {
	MockRefCountedObject o;
	EXPECT_EQ(o.ReferenceCount(), 1);
	o.Duplicate();
	EXPECT_EQ(o.ReferenceCount(), 2);
	o.Duplicate();
	EXPECT_EQ(o.ReferenceCount(), 3);
	o.Release();
	EXPECT_EQ(o.ReferenceCount(), 2);
	o.Release();
	EXPECT_EQ(o.ReferenceCount(), 1);
}

TEST(RefCountedObjectTestSuite, ref_ptr) {
	MockRefCountedObject * o = new MockRefCountedObject();
	EXPECT_EQ(o->ReferenceCount(), 1);
	o->Duplicate();
	EXPECT_EQ(o->ReferenceCount(), 2);
	o->Release();
	EXPECT_EQ(o->ReferenceCount(), 1);
	o->Release();
}

TEST(RefCountedObjectTestSuite, del) {
	MockRefCountedObject * o = (MockRefCountedObject *)jc_malloc(sizeof(*o));
	new(o)MockRefCountedObject();
	EXPECT_EQ(o->ReferenceCount(), 1);
	o->Duplicate();
	EXPECT_EQ(o->ReferenceCount(), 2);
	o->Release();
	o->Release();
}
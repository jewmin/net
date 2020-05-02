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
	Net::RefCounter counter;
	EXPECT_EQ((int)counter, 1);
	EXPECT_EQ((int)counter++, 1);
	EXPECT_EQ((int)++counter, 3);
	EXPECT_EQ((int)--counter, 2);
	EXPECT_EQ((int)counter--, 2);
	EXPECT_EQ((int)counter, 1);
}

TEST(RefCountedObjectTestSuite, ref) {
	MockRefCountedObject ref_object;
	EXPECT_EQ(ref_object.ReferenceCount(), 1);
	ref_object.Duplicate();
	EXPECT_EQ(ref_object.ReferenceCount(), 2);
	ref_object.Duplicate();
	EXPECT_EQ(ref_object.ReferenceCount(), 3);
	ref_object.Release();
	EXPECT_EQ(ref_object.ReferenceCount(), 2);
	ref_object.Release();
	EXPECT_EQ(ref_object.ReferenceCount(), 1);
}

TEST(RefCountedObjectTestSuite, ref_ptr) {
	MockRefCountedObject * ref_object = new MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Duplicate();
	EXPECT_EQ(ref_object->ReferenceCount(), 2);
	ref_object->Release();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Release();
}

TEST(RefCountedObjectTestSuite, del) {
	char buf[sizeof(MockRefCountedObject)];
	MockRefCountedObject * ref_object = reinterpret_cast<MockRefCountedObject *>(const_cast<char *>(buf));
	new(ref_object)MockRefCountedObject();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
	ref_object->Duplicate();
	EXPECT_EQ(ref_object->ReferenceCount(), 2);
	ref_object->Release();
	EXPECT_EQ(ref_object->ReferenceCount(), 1);
}
#include "gtest/gtest.h"
#include "RefCountedObject.h"

using namespace Net;

class RefCountedObjectTestSuite_TestRefObj : public RefCountedObject {
public:
	RefCountedObjectTestSuite_TestRefObj() : value_(0) {

	}
	virtual ~RefCountedObjectTestSuite_TestRefObj() {
		
	}

	int GetValue() const {
		return value_;
	}

	void SetValue(int value) {
		value_ = value;
	}

private:
	int value_;
};

TEST(RefCountedObjectTestSuite, RefCounter) {
	RefCounter counter;
	EXPECT_TRUE(counter == 1);
	EXPECT_TRUE(counter++ == 1);
	EXPECT_TRUE(++counter == 3);
	EXPECT_TRUE(counter-- == 3);
	EXPECT_TRUE(--counter == 1);
}

TEST(RefCountedObjectTestSuite, RefCountedObject) {
	RefCountedObjectTestSuite_TestRefObj obj;
	EXPECT_EQ(1, obj.ReferenceCount());
	obj.Duplicate();
	EXPECT_EQ(2, obj.ReferenceCount());
	obj.Release();
	EXPECT_EQ(1, obj.ReferenceCount());
}

TEST(RefCountedObjectTestSuite, RefCountedObjectPtr) {
	RefCountedObjectTestSuite_TestRefObj * obj = new RefCountedObjectTestSuite_TestRefObj();
	EXPECT_EQ(1, obj->ReferenceCount());
	obj->Release();
}

TEST(RefCountedObjectTestSuite, RefPtr) {
	RefCountedObjectTestSuite_TestRefObj * ptr = nullptr;
	const RefCountedObjectTestSuite_TestRefObj * ptr2 = nullptr;
	RefCountedObjectTestSuite_TestRefObj * ptr3 = new RefCountedObjectTestSuite_TestRefObj();
	const RefCountedObjectTestSuite_TestRefObj * ptr4 = new RefCountedObjectTestSuite_TestRefObj();
	RefCountedObjectTestSuite_TestRefObj * ptr5 = new RefCountedObjectTestSuite_TestRefObj();
	const RefCountedObjectTestSuite_TestRefObj * ptr6 = new RefCountedObjectTestSuite_TestRefObj();

	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj1;
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj2(ptr);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj3(ptr3);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj4(const_cast<RefCountedObjectTestSuite_TestRefObj *>(ptr4));
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj5(obj1);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj6(obj2);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj7(obj3);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj8(obj4);
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj9, obj10, obj11, obj12, obj13, obj14, obj15, obj16;

	obj9 = obj1;
	obj10 = obj2;
	obj11 = obj3;
	obj12 = obj4;
	obj13 = ptr;
	obj14 = const_cast<RefCountedObjectTestSuite_TestRefObj *>(ptr2);
	obj15 = ptr5;
	obj16 = const_cast<RefCountedObjectTestSuite_TestRefObj *>(ptr6);

	EXPECT_TRUE(!obj1);

	EXPECT_TRUE(obj1 == ptr);
	EXPECT_TRUE(obj2 == ptr2);
	EXPECT_TRUE(obj3 == obj7);
	EXPECT_TRUE(obj1 == obj9);

	EXPECT_TRUE(obj1 != ptr3);
	EXPECT_TRUE(obj2 != ptr4);
	EXPECT_TRUE(obj3 != obj8);
	EXPECT_TRUE(obj4 != obj10);

	EXPECT_TRUE(obj1 < ptr3);
	EXPECT_TRUE(obj2 < ptr4);
	EXPECT_TRUE(obj2 < obj8);

	EXPECT_TRUE(obj1 <= ptr);
	EXPECT_TRUE(obj2 <= ptr2);
	EXPECT_TRUE(obj3 <= obj7);
	EXPECT_TRUE(obj9 <= obj10);

	EXPECT_TRUE(obj3 > ptr);
	EXPECT_TRUE(obj4 > ptr2);
	EXPECT_TRUE(obj8 > obj2);

	EXPECT_TRUE(obj1 >= ptr);
	EXPECT_TRUE(obj2 >= ptr2);
	EXPECT_TRUE(obj7 >= obj3);
	EXPECT_TRUE(obj9 >= obj10);
}

TEST(RefCountedObjectTestSuite, RefPtrRRef) {
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj1(new RefCountedObjectTestSuite_TestRefObj());
	RefCountedObjectTestSuite_TestRefObj * ptr1 = obj1.Duplicate();
	EXPECT_EQ(ptr1->ReferenceCount(), 2);
	obj1->Release();
	EXPECT_EQ((*obj1).ReferenceCount(), 1);
	RefCountedObjectTestSuite_TestRefObj * ptr2 = obj1.Get();
	RefCountedObjectTestSuite_TestRefObj * ptr3 = static_cast<RefCountedObjectTestSuite_TestRefObj *>(obj1);
	EXPECT_TRUE(ptr3 == ptr2);

	RefCountedObjectTestSuite_TestRefObj * ptr4 = new RefCountedObjectTestSuite_TestRefObj();
	const RefPtr<RefCountedObjectTestSuite_TestRefObj> obj5(ptr4);
	const RefCountedObjectTestSuite_TestRefObj * ptr5 = obj5.Get();
	const RefCountedObjectTestSuite_TestRefObj * ptr6 = static_cast<const RefCountedObjectTestSuite_TestRefObj *>(obj5);
	EXPECT_TRUE(ptr5 == ptr6);
	EXPECT_EQ(obj5->ReferenceCount(), (*obj5).ReferenceCount());
}

TEST(RefCountedObjectTestSuite, RefPtrDeref) {
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj1(new RefCountedObjectTestSuite_TestRefObj());
	obj1->SetValue(100);
	EXPECT_EQ(obj1->GetValue(), 100);
}

TEST(RefCountedObjectTestSuite, RefPtrReset) {
	RefPtr<RefCountedObjectTestSuite_TestRefObj> obj1(new RefCountedObjectTestSuite_TestRefObj()), obj2;
	obj2.Swap(obj1);
	EXPECT_EQ(obj2->ReferenceCount(), 1);
	EXPECT_TRUE(obj1.Get() == nullptr);
}

TEST(RefCountedObjectTestSuite, Catch) {
	try {
		RefCountedObjectTestSuite_TestRefObj * obj = nullptr;
		RefPtr<RefCountedObjectTestSuite_TestRefObj> ptr(obj);
		ptr->GetValue();
	} catch (std::exception & e) {
		printf("RefCountedObjectTestSuite - Catch: %s\n", e.what());
	}
}

TEST(RefCountedObjectTestSuite, Assign) {
	RefPtr<RefCountedObjectTestSuite_TestRefObj> ptr(new RefCountedObjectTestSuite_TestRefObj());
	ptr = nullptr;

	RefPtr<RefCountedObjectTestSuite_TestRefObj> ptr2(new RefCountedObjectTestSuite_TestRefObj()), ptr3;
	ptr2 = ptr3;
}
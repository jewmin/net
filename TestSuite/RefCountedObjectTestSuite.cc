#include "gtest/gtest.h"
#include "RefCountedObject.h"

class TestRefObj : public Foundation::RefCountedObject {
public:
	TestRefObj() : value_(0) {

	}
	virtual ~TestRefObj() {
		
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
	Foundation::RefCounter counter;
	EXPECT_TRUE(counter == 1);
	EXPECT_TRUE(counter++ == 1);
	EXPECT_TRUE(++counter == 3);
	EXPECT_TRUE(counter-- == 3);
	EXPECT_TRUE(--counter == 1);
}

TEST(RefCountedObjectTestSuite, RefCountedObject) {
	TestRefObj obj;
	EXPECT_EQ(1, obj.ReferenceCount());
	obj.Duplicate();
	EXPECT_EQ(2, obj.ReferenceCount());
	obj.Release();
	EXPECT_EQ(1, obj.ReferenceCount());
}

TEST(RefCountedObjectTestSuite, RefCountedObjectPtr) {
	TestRefObj * obj = new TestRefObj();
	EXPECT_EQ(1, obj->ReferenceCount());
	obj->Release();
}

TEST(RefCountedObjectTestSuite, RefPtr) {
	Foundation::RefPtr<TestRefObj> obj1;
	Foundation::RefPtr<TestRefObj> obj2(new TestRefObj());
	TestRefObj * ptr = new TestRefObj();
	Foundation::RefPtr<TestRefObj> obj6(ptr, true);
	Foundation::RefPtr<TestRefObj> obj3(obj1);
	Foundation::RefPtr<TestRefObj> obj4(obj2);
	Foundation::RefPtr<TestRefObj> obj5(new TestRefObj());
	const TestRefObj * ptr2 = new TestRefObj();
	Foundation::RefPtr<TestRefObj> obj7(const_cast<TestRefObj *>(ptr2));
	Foundation::RefPtr<TestRefObj> obj8(new TestRefObj());
	const TestRefObj * ptr3 = nullptr;
	TestRefObj * ptr4 = nullptr;
	
	EXPECT_TRUE(obj1 != obj2);
	EXPECT_TRUE(obj1 == obj3);
	EXPECT_TRUE(obj2 != obj3);
	EXPECT_TRUE(obj2 == obj4);
	EXPECT_TRUE(obj1 != obj2);

	EXPECT_TRUE(obj1 < obj2);
	EXPECT_TRUE(obj1 <= obj3);
	EXPECT_TRUE(obj5 > obj1);
	EXPECT_TRUE(obj4 >= obj2);

	EXPECT_TRUE(obj1 != ptr);
	EXPECT_TRUE(obj6 == ptr);
	EXPECT_TRUE(obj3 < ptr);
	EXPECT_TRUE(obj5 > ptr4);
	EXPECT_TRUE(obj6 >= ptr);
	EXPECT_TRUE(obj6 <= ptr);

	EXPECT_TRUE(obj1 != ptr2);
	EXPECT_TRUE(obj7 == ptr2);
	EXPECT_TRUE(obj3 < ptr2);
	EXPECT_TRUE(obj8 > ptr3);
	EXPECT_TRUE(obj7 >= ptr2);
	EXPECT_TRUE(obj7 <= ptr2);

	EXPECT_TRUE(!obj1);

	obj6->Release();
}

TEST(RefCountedObjectTestSuite, RefPtrRRef) {
	Foundation::RefPtr<TestRefObj> obj1(new TestRefObj());
	Foundation::RefPtr<TestRefObj> obj2(std::move(obj1));
	Foundation::RefPtr<TestRefObj> obj3, obj4(new TestRefObj());
	obj3 = std::move(obj2);
	obj4 = std::move(obj3);
	TestRefObj * ptr = new TestRefObj();
	obj4.Assign(ptr, true);
	EXPECT_EQ(ptr->ReferenceCount(), 2);
	TestRefObj * ptr2 = obj4.Duplicate();
	EXPECT_TRUE(ptr2 == ptr);
	EXPECT_EQ(ptr2->ReferenceCount(), 3);
	obj4->Release();
	obj4->Release();
	EXPECT_EQ((*obj4).ReferenceCount(), 1);
	TestRefObj * ptr3 = obj4.Get();
	TestRefObj * ptr4 = static_cast<TestRefObj *>(obj4);
	EXPECT_TRUE(ptr4 == ptr3);

	TestRefObj * ptr5 = new TestRefObj();
	const Foundation::RefPtr<TestRefObj> obj5(ptr5);
	const TestRefObj * ptr6 = obj5.Get();
	const TestRefObj * ptr7 = static_cast<const TestRefObj *>(obj5);
	EXPECT_TRUE(ptr6 == ptr7);
	EXPECT_EQ(obj5->ReferenceCount(), (*obj5).ReferenceCount());
}

TEST(RefCountedObjectTestSuite, RefPtrAssign) {
	Foundation::RefPtr<TestRefObj> obj1(new TestRefObj());
	Foundation::RefPtr<TestRefObj> obj2;
	obj2 = obj1;
	TestRefObj * ptr1 = new TestRefObj();
	obj2 = ptr1;
}

TEST(RefCountedObjectTestSuite, RefPtrDeref) {
	Foundation::RefPtr<TestRefObj> obj1(new TestRefObj());
	obj1->SetValue(100);
	EXPECT_EQ(obj1->GetValue(), 100);
}

TEST(RefCountedObjectTestSuite, RefPtrReset) {
	Foundation::RefPtr<TestRefObj> obj1(new TestRefObj()), obj2;
	obj2.Swap(obj1);
	EXPECT_EQ(obj2->ReferenceCount(), 1);
	obj1.Reset(obj2);
	EXPECT_EQ(obj1->ReferenceCount(), 2);
	obj2.Reset(obj1.Get());
	EXPECT_EQ(obj1->ReferenceCount(), 2);
	obj1.Reset(obj2.Get(), true);
	EXPECT_EQ(obj1->ReferenceCount(), 2);
}
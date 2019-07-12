#include "gtest/gtest.h"
#include "SharedPtr.h"

class TestObject {
public:
	TestObject() : value_(0) {

	}

	TestObject(int value) : value_(value) {

	}

	~TestObject() {

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

typedef Foundation::SharedPtr<TestObject> TestObjectPtr;

TEST(SharedPtrTestSuite, Constructor) {
	std::shared_ptr<TestObject> sptr(new TestObject(3));
	const TestObjectPtr ptr1, ptr2(new TestObject(2)), ptr3(sptr);
	TestObjectPtr ptr4(ptr2), ptr5(std::move(TestObjectPtr(new TestObject(5)))), ptr6(std::make_shared<TestObject>(6));
	TestObjectPtr ptr7(new TestObject(7)), ptr8, ptr9, ptr10, ptr11;

	ptr8 = new TestObject(8);
	ptr9 = ptr6;
	ptr10 = std::move(ptr7);
	ptr11 = std::move(std::make_shared<TestObject>(11));

	EXPECT_EQ(ptr1.ReferenceCount(), 0);
	EXPECT_EQ(ptr2.ReferenceCount(), 2);
	EXPECT_EQ(ptr3.ReferenceCount(), 2);
	EXPECT_EQ(ptr4.ReferenceCount(), 2);
	EXPECT_EQ(ptr5.ReferenceCount(), 1);
	EXPECT_EQ(ptr6.ReferenceCount(), 2);
	EXPECT_EQ(ptr7.ReferenceCount(), 0);
	EXPECT_EQ(ptr8.ReferenceCount(), 1);
	EXPECT_EQ(ptr9.ReferenceCount(), 2);
	EXPECT_EQ(ptr10.ReferenceCount(), 1);
	EXPECT_EQ(ptr11.ReferenceCount(), 1);
}

TEST(SharedPtrTestSuite, Reset) {
	const TestObjectPtr ptr1(new TestObject(1));
	TestObjectPtr ptr2(new TestObject(2)), ptr3, ptr4;
	ptr2.Reset();
	ptr3.Reset(new TestObject(3));
	ptr4.Reset(ptr1);

	EXPECT_EQ(ptr1.ReferenceCount(), 2);
	EXPECT_EQ(ptr2.ReferenceCount(), 0);
	EXPECT_EQ(ptr3.ReferenceCount(), 1);
	EXPECT_EQ(ptr4.ReferenceCount(), 2);
}

TEST(SharedPtrTestSuite, Get) {
	const TestObjectPtr ptr1(new TestObject(1));
	TestObjectPtr ptr2(new TestObject(2));
	EXPECT_EQ(ptr1->GetValue(), 1);
	EXPECT_EQ(ptr2->GetValue(), 2);
	EXPECT_EQ((*ptr1).GetValue(), 1);
	EXPECT_EQ((*ptr2).GetValue(), 2);
	EXPECT_EQ(ptr1.Get()->GetValue(), 1);
	EXPECT_EQ(ptr2.Get()->GetValue(), 2);
	EXPECT_EQ(static_cast<const TestObject *>(ptr1)->GetValue(), 1);
	EXPECT_EQ(static_cast<TestObject *>(ptr2)->GetValue(), 2);
}

TEST(SharedPtrTestSuite, Compare) {
	TestObject * obj1 = new TestObject(1);
	const TestObject * obj2 = new TestObject(2);
	const TestObjectPtr ptr1(obj1);
	TestObjectPtr ptr2(const_cast<TestObject *>(obj2)), ptr4(ptr2);
	TestObjectPtr ptr3;

	TestObject * obj3 = new TestObject();
	TestObject * obj4 = new TestObject();
	if (obj3 > obj4) {
		TestObject * tmp = obj3;
		obj3 = obj4;
		obj4 = tmp;
	}
	obj3->SetValue(3);
	obj4->SetValue(4);
	const TestObject * obj5 = obj3;
	const TestObject * obj6 = obj4;
	TestObjectPtr low(obj3), high(obj4);

	EXPECT_FALSE(!ptr1);
	EXPECT_FALSE(!ptr2);
	EXPECT_TRUE(!ptr3);

	EXPECT_TRUE(ptr1 == obj1);
	EXPECT_TRUE(ptr2 == obj2);
	EXPECT_TRUE(ptr2 == ptr4);

	EXPECT_TRUE(ptr1 != obj2);
	EXPECT_TRUE(ptr2 != obj1);
	EXPECT_TRUE(ptr2 != ptr3);

	EXPECT_TRUE(low < obj4);
	EXPECT_TRUE(low < obj6);
	EXPECT_TRUE(low < high);

	EXPECT_TRUE(low <= obj4);
	EXPECT_TRUE(low <= obj6);
	EXPECT_TRUE(low <= high);
	EXPECT_TRUE(low <= obj3);
	EXPECT_TRUE(low <= obj5);
	EXPECT_TRUE(low <= low);

	EXPECT_TRUE(high > obj3);
	EXPECT_TRUE(high > obj5);
	EXPECT_TRUE(high > low);

	EXPECT_TRUE(high >= obj3);
	EXPECT_TRUE(high >= obj5);
	EXPECT_TRUE(high >= low);
	EXPECT_TRUE(high >= obj4);
	EXPECT_TRUE(high >= obj6);
	EXPECT_TRUE(high >= high);
}
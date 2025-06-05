#include <gtest/gtest.h>

TEST(SampleTest, BasicAssertions) {
	// Expect two strings to be equal.
	EXPECT_STREQ("hello", "hello");
	// Expect equality.
	EXPECT_EQ(7 * 6, 42);
}
#include <gtest/gtest.h>
#include "SlimUtil.h"

TEST(SlimUtil, CanCreateEmptyString)
{
    const char* actual = CSlim_CreateEmptyString();
    EXPECT_STREQ("", actual);
    CSlim_DestroyString(actual);
}

TEST(SlimUtil, CanConcatenateToAnEmptyString)
{
    const char* actual = CSlim_CreateEmptyString();
    CSlim_ConcatenateString(&actual, "a");
    EXPECT_STREQ("a", actual);
    CSlim_DestroyString(actual);
}

TEST(SlimUtil, CanConcatenateToANonEmptyString)
{
    const char* actual = CSlim_CreateEmptyString();
    CSlim_ConcatenateString(&actual, "a");
    CSlim_ConcatenateString(&actual, "b");
    EXPECT_STREQ("ab", actual);
    CSlim_DestroyString(actual);
}

TEST(SlimUtil, StringStartsWith)
{
    EXPECT_TRUE(CSlim_StringStartsWith("", ""));
    EXPECT_FALSE(CSlim_StringStartsWith("", "a"));
    EXPECT_FALSE(CSlim_StringStartsWith("a", "ab"));
    EXPECT_TRUE(CSlim_StringStartsWith("a", ""));
    EXPECT_TRUE(CSlim_StringStartsWith("a", "a"));
    EXPECT_TRUE(CSlim_StringStartsWith("ab", "a"));
    EXPECT_FALSE(CSlim_StringStartsWith("a", "b"));
    EXPECT_TRUE(CSlim_StringStartsWith("abc", "ab"));
    EXPECT_FALSE(CSlim_StringStartsWith("abc", "ac"));
}

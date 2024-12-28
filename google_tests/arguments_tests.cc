#include <gtest/gtest.h>
#include "../arguments.h"

TEST(ParseArgumentsTest, NoArgs) {
    const char *argv[] = {"program"};
    const Arguments* args = parseArguments(1, argv);

    EXPECT_TRUE(args->error);
    EXPECT_EQ(nullptr, args->dst_adf);
    EXPECT_EQ(nullptr, args->src_folder);
    EXPECT_EQ(0, args->exclusion_count);
    EXPECT_EQ(nullptr, args->exclusions);
}

TEST(ParseArgumentsTest, NoSource) {
    const char *argv[] = {"program", "-d", "some.adf"};
    const Arguments* args = parseArguments(1, argv);

    EXPECT_TRUE(args->error);
    EXPECT_EQ(nullptr, args->dst_adf);
    EXPECT_EQ(nullptr, args->src_folder);
    EXPECT_EQ(0, args->exclusion_count);
    EXPECT_EQ(nullptr, args->exclusions);
}

TEST(ParseArgumentsTest, NoDestination) {
    const char *argv[] = {"program", "-s", "c:\\source"};
    const Arguments* args = parseArguments(1, argv);

    EXPECT_TRUE(args->error);
    EXPECT_EQ(nullptr, args->dst_adf);
    EXPECT_EQ(nullptr, args->src_folder);
    EXPECT_EQ(0, args->exclusion_count);
    EXPECT_EQ(nullptr, args->exclusions);
}

TEST(ParseArgumentsTest, AllRequiredArguments) {
    const char *argv[] = {"program", "-s", "c:\\source", "-d", "some.adf"};
    const Arguments* args = parseArguments(1, argv);

    EXPECT_TRUE(args->error);
    EXPECT_EQ(nullptr, args->dst_adf);
    EXPECT_EQ(nullptr, args->src_folder);
    EXPECT_EQ(0, args->exclusion_count);
    EXPECT_EQ(nullptr, args->exclusions);
}

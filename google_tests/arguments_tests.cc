#include <iterator>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "../arguments.h"

TEST(ParseArgumentsTest, NoArgs_ErrorsOut) {
    const char *argv[] = {"program"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("Missing required"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, MissingSourceValueAtTheEnd_ErrorsOut) {
    const char *argv[] = {"program", "-d", "some.adf", "-s" };
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("-s requires"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, MissingSourceValueEarlier_ErrorsOut) {
    const char *argv[] = {"program", "-s", "-d", "destination.adf" };
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("-s requires"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, MissingDestinationValueEarlier_ErrorsOut) {
    const char *argv[] = {"program", "-d", "-s", "some source" };
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("-d requires"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, MissingDestinationValueAtTheEnd_ErrorsOut) {
    const char *argv[] = {"program", "-s", "some source", "-d" };
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("-d requires"));
    freeArguments(args);
}


TEST(ParseArgumentsTest, MissingSourceArgument_ErrorsOut) {
    const char *argv[] = {"program", "-d", "some.adf"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("Missing required"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, MissingDestinationArgument_ErrorsOut) {
    const char *argv[] = {"program", "-s", "c:\\source"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("Missing required"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, AllRequiredArguments_AreReturned) {
    const char *argv[] = {"program", "-s", "c:\\source", "-d", "some.adf"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_FALSE(args->error);
    EXPECT_EQ("some.adf", args->dst_adf);
    EXPECT_EQ("c:\\source", args->src_folder);
    freeArguments(args);
}

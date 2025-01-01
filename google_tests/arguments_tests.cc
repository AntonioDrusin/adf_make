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
    EXPECT_STREQ("some.adf", args->dst_adf);
    EXPECT_STREQ("c:\\source", args->src_folder);
    freeArguments(args);
}

TEST(ParseArgumentsTest, AllArguments_AreReturned) {
    const char *argv[] = {"program", "-s", "c:\\source", "-d", "some.adf", "-l", "Label"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_FALSE(args->error);
    EXPECT_STREQ("some.adf", args->dst_adf);
    EXPECT_STREQ("c:\\source", args->src_folder);
    EXPECT_STREQ("Label", args->label);
    freeArguments(args);
}

TEST(ParseArgumentsTest, WhenLabelMissing_ErrorsOut) {
    const char *argv[] = {"program", "-l", "-s", "some source", "-d", "dest.adf" };
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_TRUE(args->error);
    EXPECT_THAT(args->error_message, testing::HasSubstr("-l requires"));
    freeArguments(args);
}

TEST(ParseArgumentsTest, WhenLabelSet_IsReturned) {
    const char *argv[] = {"program", "-s", "some source", "-l", "New Label", "-d", "dest.adf"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_FALSE(args->error);
    EXPECT_STREQ("New Label", args->label);
    freeArguments(args);
}

TEST(ParseArgumentsTest, WhenLabelNotSet_DefaultIsReturned) {
    const char *argv[] = {"program", "-s", "some source", "-d", "dest.adf"};
    const Arguments* args = getArguments( std::ssize(argv), argv);

    EXPECT_FALSE(args->error);
    EXPECT_STREQ("empty", args->label);
    freeArguments(args);
}


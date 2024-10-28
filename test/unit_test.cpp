#include <iostream>
#include <gtest/gtest.h>
#include "../include/json.h"

using namespace std;
using namespace ty;

#define JSON_PARSE_TEST_LIBERAL(jstr, jtype)             \
    do                                                   \
    {                                                    \
        json v = json::parse(jstr);                      \
        ASSERT_EQ(jtype, v.get_type()) << v.get_error(); \
    } while (0)

#define JSON_PARSE_TEST_NUMBER(jstr, jtype, jvalue)         \
    do                                                      \
    {                                                       \
        json v = json::parse(jstr);                         \
        ASSERT_EQ(jtype, v.get_type()) << v.get_error();    \
        EXPECT_DOUBLE_EQ(jvalue, v.template get<double>()); \
    } while (0)

#define JSON_PARSE_TEST_STRING(jstr, jtype, jvalue)           \
    do                                                        \
    {                                                         \
        json v = json::parse(jstr);                           \
        ASSERT_EQ(jtype, v.get_type()) << v.get_error();      \
        EXPECT_STREQ(jvalue, v.template get<const char *>()); \
    } while (0)

TEST(json_parse, json_null)
{
    JSON_PARSE_TEST_LIBERAL("null", JSON_NULL);
}

TEST(json_parse, json_true)
{
    JSON_PARSE_TEST_LIBERAL("true", JSON_TRUE);
}

TEST(json_parse, json_false)
{
    JSON_PARSE_TEST_LIBERAL("false", JSON_FALSE);
}

TEST(json_parse, json_number)
{
    JSON_PARSE_TEST_NUMBER("123456", ty::json_type::JSON_NUMBER, 123456);
    JSON_PARSE_TEST_NUMBER("100.24", ty::json_type::JSON_NUMBER, 100.24);
    JSON_PARSE_TEST_NUMBER("0.000001", ty::json_type::JSON_NUMBER, 0.000001);
    JSON_PARSE_TEST_NUMBER("12.434E8", ty::json_type::JSON_NUMBER, 12.434E8);
    JSON_PARSE_TEST_NUMBER("17462992e-9", ty::json_type::JSON_NUMBER, 17462992e-9);
    JSON_PARSE_TEST_NUMBER("1.2412423e+14", ty::json_type::JSON_NUMBER, 1.2412423e+14);
}

TEST(json_parse, json_string)
{
    JSON_PARSE_TEST_STRING("\"你好！\\t接下来换一行\\n结束。\"", ty::json_type::JSON_STRING, "你好！\t接下来换一行\n结束。");
    JSON_PARSE_TEST_STRING("\"\\u4f60\\u597d\\uff01\"", ty::json_type::JSON_STRING, "\u4f60\u597d\uff01");
    JSON_PARSE_TEST_STRING("\"\\ud83d\\ude00\\uff08-\\u3002-\\uff09\\ud83d\\ude00\"", ty::json_type::JSON_STRING, "😀（-。-）😀");
}

TEST(json_parse, json_array)
{
    json v = json::parse("[true, false,null , 421.22, \"\\u4f60\\u597d\\uff01 你好!!!\", [ 1,2,3,4,5 ] ]");
    ASSERT_EQ(JSON_ARRAY, v.get_type()) << v.get_error();
    ASSERT_EQ(JSON_ARRAY, v[5].get_type()) << v.get_error();
    EXPECT_EQ(true, v[0].template get<bool>());
    EXPECT_EQ(false, v[1].template get<bool>());
    EXPECT_EQ(JSON_NULL, v[2].get_type());
    EXPECT_DOUBLE_EQ(421.22, v[3].template get<double>());
    EXPECT_STREQ("\u4f60\u597d\uff01 你好!!!", v[4].template get<const char *>());

    for (size_t i = 0; i < v[5].size(); i++)
    {
        EXPECT_DOUBLE_EQ((i + 1), v[5][i].template get<float>());
    }
}

TEST(json_parse, json_object)
{
    json v = json::parse("{\"a\":100023 , \"b\": true, \"c\":\"\\ud83d\\ude00\\uff08-\\u3002-\\uff09\\ud83d\\ude00\", \"d\":[1.01, \"OK\", 33]}");
    ASSERT_EQ(JSON_OBJECT, v.get_type()) << v.get_error();
    EXPECT_DOUBLE_EQ(100023, v["a"]);
    EXPECT_EQ(true, v["b"]);
    EXPECT_STREQ("😀（-。-）😀", v["c"]);
    ASSERT_EQ(JSON_ARRAY, v["d"].get_type()) << v.get_error();
    EXPECT_DOUBLE_EQ(1.01, v["d"][0]);
    EXPECT_STREQ("OK", v["d"][1]);
}

TEST(json_strinify, json){
    std::string json_str = "{\"a\":100023 , \"b\": true, \"c\":\"\\uD83D\\uDE00\\uFF08-\\u3002-\\uFF09\\uD83D\\uDE00\", \"d\":[1.01, \"OK是好的\", 33]}";
    json v = json::parse(json_str);
    json v2 = json::parse(v.stringify());
    EXPECT_DOUBLE_EQ(v2["a"], v["a"]);
    EXPECT_EQ(v2["b"], v["b"]);
    EXPECT_STREQ(v2["c"], v["c"]);
    EXPECT_DOUBLE_EQ(v2["d"][0], v["d"][0]);
    EXPECT_STREQ(v2["d"][1], v["d"][1]);
    EXPECT_EQ(v2, v);
    json v3 = "😀（-。-）😀";
    EXPECT_EQ(v2["c"], v3);

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
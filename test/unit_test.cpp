#include <iostream>
#include <gtest/gtest.h>
#include "../include/json.h"

using namespace std;
using namespace ty;

TEST(json_parse, null){
    json j;
    json_value* v = j.parse("null");
    EXPECT_EQ(JSON_NULL, v->type);
    free(v);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS(); 
}
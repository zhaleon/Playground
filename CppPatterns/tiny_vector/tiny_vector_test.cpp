#include <gtest/gtest.h>

#include "tiny_vector.hpp"

class TinyVectorTest : public ::testing::Test {

protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST(TinyVectorTest, BasicTest) {
    tiny::vector<int> v;
    for (int i = 0; i < 10000; ++i) {
        v.push_back(i);
    }
}

TEST(TinyVectorTest, DoubleMoveLeakTest) {
    tiny::vector<int> a;
    for (int i = 0; i < 10000; ++i) {
        a.push_back(i);
    }

    auto b = std::move(a);
    auto c = std::move(b);
}


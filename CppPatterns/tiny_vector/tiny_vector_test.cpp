#include <gtest/gtest.h>
#include "fuzztest/fuzztest.h"

#include "tiny_vector.hpp"

#include <memory>

// bazel test --config=asan --test_output=all tiny_vector_test --jobs=32

class TinyVectorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST(TinyVectorTest, BasicTest) {
    const int N_ELEMS = 100000;
    tiny::vector<int> v;
    for (int i = 0; i < N_ELEMS; ++i) {
        v.push_back(i);
    }

    // test size
    EXPECT_EQ(v.size(), N_ELEMS);

    // test operator[] and .at()
    EXPECT_EQ(v[999], 999);
    EXPECT_EQ(v.at(999), 999);

    // test access
    EXPECT_EQ(v.front(), 0);
    EXPECT_EQ(v.back(), N_ELEMS - 1);

    // test pop_back()
    v.pop_back();
    EXPECT_EQ(v.size(), N_ELEMS - 1);

    // check const iterators and iterators
    for (const int i : v) {
        EXPECT_EQ(i, i);
    }

    for (int i : v) {
        EXPECT_EQ(i, i);
    }

    for (int i = 0; i < N_ELEMS; ++i) {
        v.emplace_back(i);
    }
}

TEST(TinyVectorTest, MoveFreeTest) {
    tiny::vector<int> a;
    for (int i = 0; i < 10000; ++i) {
        a.push_back(i);
    }

    auto b = std::move(a);
    auto c = std::move(b);
}

TEST(TinyVectorTest, SwapTest) {
    const int N_ELEMENTS = 10000;
    tiny::vector<int> a;
    for (int i = 0; i < N_ELEMENTS; ++i) {
        a.push_back(i);
    }

    tiny::vector<int> b;

    EXPECT_EQ(a.size(), N_ELEMENTS);
    EXPECT_EQ(b.size(), 0);

    a.swap(b);

    EXPECT_EQ(a.size(), 0);
    EXPECT_EQ(b.size(), N_ELEMENTS);
    EXPECT_EQ(b[999], 999);

    a.swap(b);

    EXPECT_EQ(a.size(), N_ELEMENTS);
    EXPECT_EQ(b.size(), 0);
    EXPECT_EQ(a[999], 999);
}

class NoDefault {
public:
    int x;
    NoDefault(int val) : x(val) {}
};

TEST(TinyVectorTest, NoDefaultConstructor) {
    tiny::vector<NoDefault> v;
    v.push_back(NoDefault(0));
    v.push_back(NoDefault(1));
    v.push_back(NoDefault(2));
    v.push_back(NoDefault(3));
    v.push_back(NoDefault(4));

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(v[i].x, i);
    }

    auto a = std::move(v);
}

TEST(TinyVectorTest, NoDoubleFreeMove) {
    tiny::vector<std::unique_ptr<int>> v;
    for (int i = 0; i < 10000; ++i) {
        v.push_back(std::make_unique<int>(i));
    }

    auto a = std::move(v);
}

TEST(TinyVectorTest, UncopyableContainerElement) {
    tiny::vector<std::unique_ptr<int>> v;
    for (int i = 0; i < 10000; ++i) {
        v.push_back(std::make_unique<int>(i));
    }
}

TEST(TinyVectorTest, AtSizeThrowsError) {
    tiny::vector<int> v;
    v.push_back(0);
    EXPECT_THROW(v.at(1), std::out_of_range);
}

TEST(TinyVectorTest, EmptyPopThrows) {
    tiny::vector<int> v;
    EXPECT_THROW(v.pop_back(), std::runtime_error);
}

TEST(TinyVectorTest, BadResizeThrows) {
    tiny::vector<int> v;
    v.push_back(0);
    v.push_back(1);
    EXPECT_THROW(v.resize(1), std::out_of_range);
}

void FuzzTest(int i) {
    EXPECT_EQ(i, i);
}

FUZZ_TEST(TinyVectorFuzzTest, FuzzTest).WithDomains(fuzztest::Positive<int>());

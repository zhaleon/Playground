cc_library(
    name = "tiny_vector",
    hdrs = ["tiny_vector.hpp"],
    srcs = [],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "tiny_vector_test",
    srcs = ["tiny_vector_test.cpp"],
    deps = [
        ":tiny_vector",
        "@googletest//:gtest_main"
    ],
    linkopts = [
        "-fsanitize=address",
        "-fsanitize=undefined"
    ],
    size = "small",
)

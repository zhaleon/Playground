#!/usr/bin/env bash
set -euo pipefail

bazel build //tiny:benchmark:tiny_vector_bench
perf record -o perf.data -g -- bazel-bin/tiny/benchmark/tiny_vector_bench \
  --size=8M --iters=10000

perf report --demangle --stdio < perf.data

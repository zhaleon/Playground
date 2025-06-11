#!/usr/bin/env bash
set -euo pipefail

bazel build //tiny_vector:tiny_vector_perf_bench

sudo perf record -o perf.data -g -- ../bazel-bin/tiny_vector/tiny_vector_perf_bench \
  --size=8M --iters=10000
sudo chown "$USER":"$USER" perf.data
perf report --demangle --stdio < perf.data > perf_report.txt

sudo perf stat \
         -o perf_stats.txt \
         -r 10 \
         -e branch-misses,cache-references,cache-misses,cycles,instructions \
         ../bazel-bin/tiny_vector/tiny_vector_perf_bench --size=8M --iters=10000

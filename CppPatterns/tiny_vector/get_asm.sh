clang++ -std=c++23 \
    -O2 \
    -fverbose-asm \
    -S \
    -target x86_64-apple-macos12.0 \
    -xc++ \
    -fkeep-inline-functions \
    -fkeep-static-consts \
    tiny_vector_inst.cpp


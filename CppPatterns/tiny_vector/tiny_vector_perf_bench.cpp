#include "tiny_vector.hpp"
#include <stdlib.h>

int main() {
    tiny::vector<char> v;

    const int n_ops = 8 * 1024 * 1024;

    for (int i = 0; i < n_ops; ++i) {
        int s = rand() % 2;
        if (s == 0 && v.size() > 0) {
            v.pop_back();
        } else {
            v.push_back(i & 0xFF);
        }
    }

    return 0;
}


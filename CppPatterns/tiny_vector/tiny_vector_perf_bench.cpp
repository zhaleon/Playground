#include "tiny_vector.hpp"
#include <stdlib.h>
#include <ctime>
#include <vector>

void random_push_pop() {
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
}

int main() {
    srand(time(nullptr));
    random_push_pop();


    tiny::vector<std::unique_ptr<int>> v, w;
    /* std::vector<std::unique_ptr<int>> v, w; */
    v.push_back(std::make_unique<int>(0));

    /* w = v; */

    return 0;
}


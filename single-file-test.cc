#include <algorithm>
#include <iostream>
#include <vector>

#include "au.hh"

using namespace au;

template <typename ExpectedT, typename ActualT>
bool expect_equal(ExpectedT expected, ActualT actual) {
    if (expected != actual) {
        std::cerr << "Failure!  Expected (" << expected << "); Actual (" << actual << ")"
                  << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    const std::vector<bool> results{
        {
            expect_equal((meters / second)(6) * seconds(6), meters(30)),
        },
    };
    return std::all_of(std::begin(results), std::end(results), [](auto x) { return x; }) ? 0 : 1;
}

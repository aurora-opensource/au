// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include <sstream>
#include <string>

namespace au {

template <typename T>
std::string stream_to_string(const T &x) {
    std::stringstream ss;
    ss << x;
    return ss.str();
}

}  // namespace au

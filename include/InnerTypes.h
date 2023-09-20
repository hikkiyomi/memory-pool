#pragma once

#include <cinttypes>

template<typename T>
struct InnerTypes {
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
};

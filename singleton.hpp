#pragma once

#include "atexit.hpp"
#include "x86.hpp"

template <typename T> class Singleton
{
public:
    static T &get(void)
    {
        static T globalClass;
        return globalClass;
    }
};

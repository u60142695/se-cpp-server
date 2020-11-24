#ifndef CPP20_UTIL_H
#define CPP20_UTIL_H

#include <memory>

class CPP20Util
{
public:
    template<typename T>
    static inline std::shared_ptr<T> MakeArray(int size)
    {
        return std::shared_ptr<T>(new T[size], [](T* p) { delete[] p; });
    }
};

#endif

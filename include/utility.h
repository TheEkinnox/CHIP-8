#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

using byte = uint8_t;

inline constexpr size_t MAX_OSPATH = 256;

inline char* com_strcpy(char *dest, size_t size, const char *src)
{
    size_t idx = 0;
    while (idx++ < size && ((*dest++ = *src++))) {}
    return dest;
}

template <size_t N>
char* com_strcpy(char (&dest)[N], const char *src)
{
    return com_strcpy(dest, N, src);
}

inline const char* com_getFilename(const char* path)
{
    if (!path)
        return nullptr;

    const size_t len = strlen(path);

    for (size_t i = len; i > 0; --i)
        if (path[i - 1] == '/' || path[i - 1] == '\\')
            return path + i;

    return path;
}

template <typename T, typename U, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr T com_setBits(const T out, const byte offset, const byte count, const U value)
{
    if (offset > sizeof(out) * CHAR_BIT - 1)
        return out;

    const T mask = (T(1) << count) - 1;
    return static_cast<T>((out & ~(mask << offset)) | ((static_cast<T>(value) & mask) << offset));
}
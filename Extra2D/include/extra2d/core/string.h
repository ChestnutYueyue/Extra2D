#pragma once

#include <string>

namespace extra2d {

// ============================================================================
// 字符串编码转换工具函数
// 统一使用 std::string (UTF-8) 作为项目标准字符串类型
// ============================================================================

// UTF-8 ↔ UTF-16 转换
std::u16string utf8ToUtf16(const std::string& utf8);
std::string utf16ToUtf8(const std::u16string& utf16);

// UTF-8 ↔ UTF-32 转换
std::u32string utf8ToUtf32(const std::string& utf8);
std::string utf32ToUtf8(const std::u32string& utf32);

// UTF-8 ↔ Wide String 转换
std::wstring utf8ToWide(const std::string& utf8);
std::string wideToUtf8(const std::wstring& wide);

// UTF-8 ↔ GBK/GB2312 转换（Windows 中文系统常用）
std::string utf8ToGbk(const std::string& utf8);
std::string gbkToUtf8(const std::string& gbk);

// ============================================================================
// 内联实现
// ============================================================================

inline std::u16string utf8ToUtf16(const std::string& utf8) {
    if (utf8.empty()) return std::u16string();

    // UTF-8 → UTF-32 → UTF-16 (with surrogate pairs)
    std::u32string u32 = utf8ToUtf32(utf8);
    std::u16string result;
    result.reserve(u32.size());

    for (char32_t ch : u32) {
        if (ch <= 0xFFFF) {
            result.push_back(static_cast<char16_t>(ch));
        } else if (ch <= 0x10FFFF) {
            // Surrogate pair
            ch -= 0x10000;
            result.push_back(static_cast<char16_t>(0xD800 | (ch >> 10)));
            result.push_back(static_cast<char16_t>(0xDC00 | (ch & 0x3FF)));
        }
    }

    return result;
}

inline std::string utf16ToUtf8(const std::u16string& utf16) {
    if (utf16.empty()) return std::string();

    // UTF-16 → UTF-32 → UTF-8
    std::u32string u32;
    u32.reserve(utf16.size());

    for (size_t i = 0; i < utf16.size(); ++i) {
        char16_t cu = utf16[i];
        char32_t ch;
        if (cu >= 0xD800 && cu <= 0xDBFF && i + 1 < utf16.size()) {
            // High surrogate
            char16_t cl = utf16[i + 1];
            if (cl >= 0xDC00 && cl <= 0xDFFF) {
                ch = 0x10000 + ((static_cast<char32_t>(cu - 0xD800) << 10) |
                                (cl - 0xDC00));
                ++i;
            } else {
                ch = cu; // Invalid, pass through
            }
        } else {
            ch = cu;
        }
        u32.push_back(ch);
    }

    return utf32ToUtf8(u32);
}

inline std::u32string utf8ToUtf32(const std::string& utf8) {
    std::u32string result;
    result.reserve(utf8.size());

    const char* ptr = utf8.c_str();
    const char* end = ptr + utf8.size();

    while (ptr < end) {
        char32_t ch = 0;
        unsigned char byte = static_cast<unsigned char>(*ptr);

        if ((byte & 0x80) == 0) {
            // 1-byte sequence
            ch = byte;
            ptr += 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte sequence
            ch = (byte & 0x1F) << 6;
            ch |= (static_cast<unsigned char>(ptr[1]) & 0x3F);
            ptr += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte sequence
            ch = (byte & 0x0F) << 12;
            ch |= (static_cast<unsigned char>(ptr[1]) & 0x3F) << 6;
            ch |= (static_cast<unsigned char>(ptr[2]) & 0x3F);
            ptr += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte sequence
            ch = (byte & 0x07) << 18;
            ch |= (static_cast<unsigned char>(ptr[1]) & 0x3F) << 12;
            ch |= (static_cast<unsigned char>(ptr[2]) & 0x3F) << 6;
            ch |= (static_cast<unsigned char>(ptr[3]) & 0x3F);
            ptr += 4;
        } else {
            // Invalid UTF-8, skip
            ptr += 1;
            continue;
        }

        result.push_back(ch);
    }

    return result;
}

inline std::string utf32ToUtf8(const std::u32string& utf32) {
    std::string result;

    for (char32_t ch : utf32) {
        if (ch <= 0x7F) {
            // 1-byte
            result.push_back(static_cast<char>(ch));
        } else if (ch <= 0x7FF) {
            // 2-byte
            result.push_back(static_cast<char>(0xC0 | ((ch >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        } else if (ch <= 0xFFFF) {
            // 3-byte
            result.push_back(static_cast<char>(0xE0 | ((ch >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        } else if (ch <= 0x10FFFF) {
            // 4-byte
            result.push_back(static_cast<char>(0xF0 | ((ch >> 18) & 0x07)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
        }
    }

    return result;
}

inline std::wstring utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();

    if constexpr (sizeof(wchar_t) == 4) {
        // wchar_t is 32-bit (Linux/Switch): same as UTF-32
        std::u32string u32 = utf8ToUtf32(utf8);
        return std::wstring(u32.begin(), u32.end());
    } else {
        // wchar_t is 16-bit (Windows): same as UTF-16
        std::u16string u16 = utf8ToUtf16(utf8);
        return std::wstring(u16.begin(), u16.end());
    }
}

inline std::string wideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();

    if constexpr (sizeof(wchar_t) == 4) {
        std::u32string u32(wide.begin(), wide.end());
        return utf32ToUtf8(u32);
    } else {
        std::u16string u16(wide.begin(), wide.end());
        return utf16ToUtf8(u16);
    }
}

// GBK/GB2312 转换（Windows 平台实现）
// 注意：Windows 实现在 .cpp 文件中，避免头文件包含 windows.h 导致冲突
#ifdef _WIN32
// 前向声明，实现在 .cpp 文件中
std::string utf8ToGbkImpl(const std::string& utf8);
std::string gbkToUtf8Impl(const std::string& gbk);

inline std::string utf8ToGbk(const std::string& utf8) {
    return utf8ToGbkImpl(utf8);
}

inline std::string gbkToUtf8(const std::string& gbk) {
    return gbkToUtf8Impl(gbk);
}
#else
// 非 Windows 平台，GBK 转换使用 iconv 或返回原字符串
inline std::string utf8ToGbk(const std::string& utf8) {
    // TODO: 使用 iconv 实现
    return utf8;
}

inline std::string gbkToUtf8(const std::string& gbk) {
    // TODO: 使用 iconv 实现
    return gbk;
}
#endif

} // namespace extra2d

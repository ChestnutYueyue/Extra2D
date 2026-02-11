#include <extra2d/core/string.h>

#ifdef _WIN32
#include <windows.h>

namespace extra2d {

std::string utf8ToGbkImpl(const std::string& utf8) {
    if (utf8.empty()) return std::string();

    // UTF-8 → Wide → GBK
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (wideLen <= 0) return std::string();

    std::wstring wide(wideLen - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], wideLen);

    int gbkLen = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (gbkLen <= 0) return std::string();

    std::string gbk(gbkLen - 1, 0);
    WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, &gbk[0], gbkLen, nullptr, nullptr);

    return gbk;
}

std::string gbkToUtf8Impl(const std::string& gbk) {
    if (gbk.empty()) return std::string();

    // GBK → Wide → UTF-8
    int wideLen = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, nullptr, 0);
    if (wideLen <= 0) return std::string();

    std::wstring wide(wideLen - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, &wide[0], wideLen);

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) return std::string();

    std::string utf8(utf8Len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], utf8Len, nullptr, nullptr);

    return utf8;
}

} // namespace extra2d

#endif // _WIN32

#pragma once

#include <cerrno>
#include <cstdio>
#include <string_view>

namespace sw::fs {

inline int clear_recent_history_file(const char *path) {
    if (!path || !*path)
        return EINVAL;
    auto *file = std::fopen(path, "w");
    if (!file)
        return errno ? errno : EIO;
    return std::fclose(file) ? (errno ? errno : EIO) : 0;
}

inline bool valid_delete_path(std::string_view path, std::string_view mountpoint) {
    if (mountpoint.empty() || mountpoint.back() != ':' || !path.starts_with(mountpoint))
        return false;

    auto internal = path.substr(mountpoint.size());
    if (internal.size() < 2 || internal.front() != '/' || internal.back() == '/' ||
            internal.find_first_of("\\:") != std::string_view::npos ||
            internal.find('\0') != std::string_view::npos)
        return false;

    internal.remove_prefix(1);
    while (!internal.empty()) {
        auto separator = internal.find('/');
        auto component = internal.substr(0, separator);
        if (component.empty() || component == "." || component == "..")
            return false;
        if (separator == std::string_view::npos)
            break;
        internal.remove_prefix(separator + 1);
    }
    return true;
}

}

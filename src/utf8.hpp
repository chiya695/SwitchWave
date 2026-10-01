#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

namespace sw::utf8 {

inline std::size_t sequence_length(std::string_view text, std::size_t offset) {
    if (offset >= text.size())
        return 0;

    auto first = static_cast<unsigned char>(text[offset]);
    std::size_t length = first < 0x80 ? 1 : first >= 0xc2 && first <= 0xdf ? 2 :
        first >= 0xe0 && first <= 0xef ? 3 : first >= 0xf0 && first <= 0xf4 ? 4 : 0;
    if (!length || offset + length > text.size())
        return 0;
    for (std::size_t index = 1; index < length; ++index)
        if ((static_cast<unsigned char>(text[offset + index]) & 0xc0) != 0x80)
            return 0;
    if (length >= 3) {
        auto second = static_cast<unsigned char>(text[offset + 1]);
        if ((first == 0xe0 && second < 0xa0) || (first == 0xed && second >= 0xa0) ||
                (first == 0xf0 && second < 0x90) || (first == 0xf4 && second >= 0x90))
            return 0;
    }
    return length;
}

inline bool valid(std::string_view text) {
    for (std::size_t offset = 0; offset < text.size();) {
        auto length = sequence_length(text, offset);
        if (!length)
            return false;
        offset += length;
    }
    return true;
}

inline int byte_offset(std::string_view text, int utf16_position) {
    std::size_t offset = 0;
    for (int units = 0; offset < text.size();) {
        auto length = std::max<std::size_t>(1, sequence_length(text, offset));
        auto next_units = length == 4 ? 2 : 1;
        if (units + next_units > utf16_position)
            break;
        units += next_units;
        offset += length;
    }
    return static_cast<int>(offset);
}

inline int utf16_offset(std::string_view text, int byte_position) {
    int units = 0;
    for (std::size_t offset = 0; offset < text.size();) {
        auto length = std::max<std::size_t>(1, sequence_length(text, offset));
        if (byte_position < 0 || offset + length > static_cast<std::size_t>(byte_position))
            break;
        units += length == 4 ? 2 : 1;
        offset += length;
    }
    return units;
}

inline int move(std::string_view text, int byte_position, int characters) {
    auto offset = static_cast<std::size_t>(byte_offset(text, utf16_offset(text, byte_position)));
    while (characters < 0 && offset > 0) {
        --offset;
        while (offset > 0 && (static_cast<unsigned char>(text[offset]) & 0xc0) == 0x80)
            --offset;
        ++characters;
    }
    while (characters > 0 && offset < text.size()) {
        offset += std::max<std::size_t>(1, sequence_length(text, offset));
        --characters;
    }
    return static_cast<int>(offset);
}

inline bool apply_keyboard_edit(std::string &text, int &cursor, std::string_view reset,
        int anchor, std::string_view input, int input_cursor) {
    if (!valid(input) || anchor < 0 || static_cast<std::size_t>(anchor) > reset.size() || input == reset)
        return false;

    std::size_t prefix = 0, suffix = 0;
    auto prefix_limit = static_cast<std::size_t>(std::min(anchor, byte_offset(input, input_cursor)));
    while (prefix < prefix_limit && prefix < input.size() &&
            reset[prefix] == input[prefix])
        ++prefix;
    while (suffix < reset.size() - anchor && suffix < input.size() - prefix &&
            reset[reset.size() - suffix - 1] == input[input.size() - suffix - 1])
        ++suffix;

    auto start = move(text, cursor, -static_cast<int>(anchor - prefix));
    auto end = move(text, cursor, static_cast<int>(reset.size() - anchor - suffix));
    auto inserted = input.substr(prefix, input.size() - prefix - suffix);
    if (text.size() - (end - start) + inserted.size() + 1 > text.capacity())
        return false;

    text.replace(start, end - start, inserted);
    cursor = start + byte_offset(inserted, std::max(0, input_cursor - static_cast<int>(prefix)));
    return true;
}

}

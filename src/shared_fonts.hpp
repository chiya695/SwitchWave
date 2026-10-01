#pragma once

#include <array>
#include <switch.h>

namespace sw {

struct SharedFont {
    PlSharedFontType type;
    const char *filename;
};

inline constexpr std::array SharedFonts = {
    SharedFont{PlSharedFontType_Standard, "nintendo_udsg-r_std_003.ttf"},
    SharedFont{PlSharedFontType_ChineseSimplified, "nintendo_chinese_simplified.ttf"},
    SharedFont{PlSharedFontType_ExtChineseSimplified, "nintendo_chinese_simplified_ext.ttf"},
    SharedFont{PlSharedFontType_ChineseTraditional, "nintendo_chinese_traditional.ttf"},
};

}

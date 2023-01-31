#pragma once

// STL
#include <string_view>

#include <QColor>

// Colors, sizes etc that are NOT derived from system

constexpr int FSZ_TABLE = 15;
constexpr int FSZ_BIG = 50;

// alpha of search/emoji border
/// @todo [hidpi] When DPI is really high, make it more opaque
constexpr int ALPHA_BORDER = 40;
// Alpha of international char’s BG
constexpr int ALPHA_INTERNATIONAL = 20;


// Color names
#define CNAME_BG_POPUP "LightYellow"
#define CNAME_LINK_POPUP "ForestGreen"
#define CNAME_LINK_DEPRECATED "#CC0000"
#define CNAME_LINK_OUTSIDE "#1565C0"        // Google blue 800
#define CNAME_U_COPY "#808080"
#define CNAME_U_BIGCOPY "rgba(128,128,128,45%)"
#define CNAME_TRANSCRIPTION "Goldenrod"
#define CNAME_ALTNAME "palette(shadow)"

constexpr QColor FG_DEPRECATED { 0xDD, 0x00, 0x00 };
constexpr QColor BG_CJK { 0xFF, 0xF0, 0xF5 };   // lavenderblush
constexpr QColor TX_CJK { 0x75, 0x50, 0x7b };   // Tango medium violet
constexpr QColor FRAME_CJK = TX_CJK;            // Format frame — same medium violet
constexpr QColor FG_CJK { 0x5c, 0x35, 0x66 };   // Tango dark violet
constexpr QColor BG_EUROPE { 0x7F, 0xFF, 0xFF };// some cyan

#define STYLE_LINK2(color1,color2,place) \
            "color:" color1 "; "   \
            "text-decoration:none; "      \
            "background:qlineargradient(x1:0, y1:1, x2:0, y2:0, " \
                    "stop:0 " color2 ", stop:" place " #00000000, stop:1 #00000000);"
#define STYLE_LINK(color,place)    STYLE_LINK2(color,color,place)
// Camouflaged link
#define STYLE_LINK_CAMO(color,place)    STYLE_LINK2("palette(window-text)",color,place)

#define STYLE_POPUP        STYLE_LINK(CNAME_LINK_POPUP, "0.05")
#define STYLE_DEPRECATED   STYLE_LINK(CNAME_LINK_DEPRECATED, "0.04")
#define STYLE_INET         STYLE_LINK(CNAME_LINK_OUTSIDE, "0.05")
#define STYLE_COPY         STYLE_LINK_CAMO(CNAME_U_COPY, "0.05")
#define STYLE_ALTNAME      STYLE_LINK2(CNAME_ALTNAME, CNAME_U_COPY, "0.05") " font-weight:bold;"
#define STYLE_BIGCOPY      STYLE_LINK_CAMO(CNAME_U_BIGCOPY, "0.05")

#define STYLES_WIKI \
        ".copy { " STYLE_COPY " } "                                             \
        ".altname { " STYLE_ALTNAME " } "                                       \
        ".bigcopy { " STYLE_BIGCOPY " } "                                       \
        ".popup { " STYLE_POPUP " } "                                           \
        ".deprecated { " STYLE_DEPRECATED " } "                                 \
        ".inet { " STYLE_INET " } "                                             \
        ".missing { color: " CNAME_LINK_DEPRECATED "; } "                       \
        ".tr { color: " CNAME_TRANSCRIPTION "; }"

#ifdef _WIN32
    #define FAM_DEFAULT "Cambria"
    #define FAM_TOFU "Times New Roman"      ///< Need just tofu glyph
    #define FAM_CONDENSED "Arial"
    #define FAM_EMOJI "Segoe UI Emoji"
#else
    #error Unknown OS
#endif

/// Alpha for space characters
constexpr int ALPHA_SPACE = 70;

/// Alpha for Egyptian hatch
constexpr int ALPHA_EGYPTIAN_HATCH = 90;

// dear imgui: wrapper to use FreeType (instead of stb_truetype)
// Get latest version at https://github.com/ocornut/imgui/tree/master/misc/freetype
// Original code by @Vuhdo (Aleksei Skriabin), maintained by @ocornut

#pragma once

#include "imgui.h"      // IMGUI_API, ImFontAtlas

namespace ImGuiFreeType
{
    typedef int FT_Error;

    // return freetype error msg for FT_Error (here replaced by int for avoid inclusion)
    const char* GetErrorMessage(FT_Error err);

    // Hinting greatly impacts visuals (and glyph sizes).
    // When disabled, FreeType generates blurrier glyphs, more or less matches the stb's output.
    // The Default hinting mode usually looks good, but may distort glyphs in an unusual way.
    // The Light hinting mode generates fuzzier glyphs but better matches Microsoft's rasterizer.

    // You can set those flags on a per font basis in ImFontConfig::RasterizerFlags.
    // Use the 'extra_flags' parameter of BuildFontAtlas() to force a flag on all your fonts.

    typedef int RasterizerFlags; // -> enum ImGuiFileDialogFlags_
    enum RasterizerFlags_
    {
        // By default, hinting is enabled and the font's native hinter is preferred over the auto-hinter.
        FreeType_NoHinting       = 1 << 0,   // Disable hinting. This generally generates 'blurrier' bitmap glyphs when the glyph are rendered in any of the anti-aliased modes.
        FreeType_NoAutoHint      = 1 << 1,   // Disable auto-hinter.
        FreeType_ForceAutoHint   = 1 << 2,   // Indicates that the auto-hinter is preferred over the font's native hinter.
        FreeType_LightHinting    = 1 << 3,   // A lighter hinting algorithm for gray-level modes. Many generated glyphs are fuzzier but better resemble their original shape. This is achieved by snapping glyphs to the pixel grid only vertically (Y-axis), as is done by Microsoft's ClearType and Adobe's proprietary font renderer. This preserves inter-glyph spacing in horizontal text.
        FreeType_MonoHinting     = 1 << 4,   // Strong hinting algorithm that should only be used for monochrome output.
        FreeType_Bold            = 1 << 5,   // Styling: Should we artificially embolden the font?
        FreeType_Oblique         = 1 << 6,   // Styling: Should we slant the font, emulating italic style?
        FreeType_Monochrome      = 1 << 7,   // Disable anti-aliasing. Combine this with MonoHinting for best results!
        FreeType_LoadColor       = 1 << 8,    // Enable FreeType color-layered glyphs
        FreeType_Default = FreeType_NoHinting | FreeType_NoAutoHint | FreeType_LoadColor
    };

    IMGUI_API bool BuildFontAtlas(ImFontAtlas* atlas, unsigned int extra_flags = 0, FT_Error* vFreetypeError = 0);

    // By default ImGuiFreeType will use IM_ALLOC()/IM_FREE().
    // However, as FreeType does lots of allocations we provide a way for the user to redirect it to a separate memory heap if desired:
    IMGUI_API void SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void* user_data = NULL);
}

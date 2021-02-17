// dear imgui: wrapper to use FreeType (instead of stb_truetype)
// Get latest version at https://github.com/ocornut/imgui/tree/master/misc/freetype
// Original code by @vuhdo (Aleksei Skriabin). Improvements by @mikesart. Maintained and v0.60+ by @ocornut.

// Changelog:
// - v0.50: (2017/08/16) imported from https://github.com/Vuhdo/imgui_freetype into http://www.github.com/ocornut/imgui_club, updated for latest changes in ImFontAtlas, minor tweaks.
// - v0.51: (2017/08/26) cleanup, optimizations, support for ImFontConfig::RasterizerFlags, ImFontConfig::RasterizerMultiply.
// - v0.52: (2017/09/26) fixes for imgui internal changes.
// - v0.53: (2017/10/22) minor inconsequential change to match change in master (removed an unnecessary statement).
// - v0.54: (2018/01/22) fix for addition of ImFontAtlas::TexUvscale member.
// - v0.55: (2018/02/04) moved to main imgui repository (away from http://www.github.com/ocornut/imgui_club)
// - v0.56: (2018/06/08) added support for ImFontConfig::GlyphMinAdvanceX, GlyphMaxAdvanceX.
// - v0.60: (2019/01/10) re-factored to match big update in STB builder. fixed texture height waste. fixed redundant glyphs when merging. support for glyph padding.
// - v0.61: (2019/01/15) added support for imgui allocators + added FreeType only override function SetAllocatorFunctions().
// - v0.62: (2019/02/09) added RasterizerFlags::Monochrome flag to disable font anti-aliasing (combine with ::MonoHinting for best results!)
// - v0.63: (2020/06/04) fix for rare case where FT_Get_Char_Index() succeed but FT_Load_Glyph() fails.
// - v0.64: (2021/01/18) add FT_Error in loading function call flag for a way for get freetype error message when bad font file
// - v0.65: (2021/01/20) add copy/past function form ImDraw and specific for COLOR support in freetype from PR : https://github.com/ocornut/imgui/pull/336, for avoid modification of ImDraw

// Gamma Correct Blending:
//  FreeType assumes blending in linear space rather than gamma space.
//  See https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_Render_Glyph
//  For correct results you need to be using sRGB and convert to linear space in the pixel shader output.
//  The default imgui styles will be impacted by this change (alpha values will need tweaking).

// FIXME: cfg.OversampleH, OversampleV are not supported (but perhaps not so necessary with this rasterizer).

#include "imgui_freetype_unleashed.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"     // ImMin,ImMax,ImFontAtlasBuild*,
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H          // <freetype/freetype.h>
#include FT_MODULE_H            // <freetype/ftmodapi.h>
#include FT_GLYPH_H             // <freetype/ftglyph.h>
#include FT_SYNTHESIS_H         // <freetype/ftsynth.h>
#include <Helper/Profiler.h>

#include <imgui/imstb_truetype.h>

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#endif

namespace
{
    // Glyph metrics:
    // --------------
    //
    //                       xmin                     xmax
    //                        |                         |
    //                        |<-------- width -------->|
    //                        |                         |
    //              |         +-------------------------+----------------- ymax
    //              |         |    ggggggggg   ggggg    |     ^        ^
    //              |         |   g:::::::::ggg::::g    |     |        |
    //              |         |  g:::::::::::::::::g    |     |        |
    //              |         | g::::::ggggg::::::gg    |     |        |
    //              |         | g:::::g     g:::::g     |     |        |
    //    offsetX  -|-------->| g:::::g     g:::::g     |  offsetY     |
    //              |         | g:::::g     g:::::g     |     |        |
    //              |         | g::::::g    g:::::g     |     |        |
    //              |         | g:::::::ggggg:::::g     |     |        |
    //              |         |  g::::::::::::::::g     |     |      height
    //              |         |   gg::::::::::::::g     |     |        |
    //  baseline ---*---------|---- gggggggg::::::g-----*--------      |
    //            / |         |             g:::::g     |              |
    //     origin   |         | gggggg      g:::::g     |              |
    //              |         | g:::::gg   gg:::::g     |              |
    //              |         |  g::::::ggg:::::::g     |              |
    //              |         |   gg:::::::::::::g      |              |
    //              |         |     ggg::::::ggg        |              |
    //              |         |         gggggg          |              v
    //              |         +-------------------------+----------------- ymin
    //              |                                   |
    //              |------------- advanceX ----------->|

    /// A structure that describe a glyph.
    struct GlyphInfo
    {
        int         Width;              // Glyph's width in pixels.
        int         Height;             // Glyph's height in pixels.
        FT_Int      OffsetX;            // The distance from the origin ("pen position") to the left of the glyph.
        FT_Int      OffsetY;            // The distance from the origin to the top of the glyph. This is usually a value < 0.
        float       AdvanceX;           // The distance from the origin to the origin of the next glyph. This is usually a value > 0.
        bool        colored;            // is this glyph is colored
    };

    // Font parameters and metrics.
    struct FontInfo
    {
        uint32_t    PixelHeight;        // Size this font was generated with.
        float       Ascender;           // The pixel extents above the baseline in pixels (typically positive).
        float       Descender;          // The extents below the baseline in pixels (typically negative).
        float       LineSpacing;        // The baseline-to-baseline distance. Note that it usually is larger than the sum of the ascender and descender taken as absolute values. There is also no guarantee that no glyphs extend above or below subsequent baselines when using this distance. Think of it as a value the designer of the font finds appropriate.
        float       LineGap;            // The spacing in pixels between one row's descent and the next row's ascent.
        float       MaxAdvanceWidth;    // This field gives the maximum horizontal cursor advance for all glyphs in the font.
    };

    // FreeType glyph rasterizer.
    // NB: No ctor/dtor, explicitly call Init()/Shutdown()
    struct FreeTypeFont
    {
        bool                    InitFont(FT_Library ft_library, const ImFontConfig& cfg, unsigned int extra_user_flags, FT_Error* vFT_Error); // Initialize from an external data buffer. Doesn't copy data, and you must ensure it stays valid up to this object lifetime.
        void                    CloseFont();
        void                    SetPixelHeight(int pixel_height); // Change font pixel size. All following calls to RasterizeGlyph() will use this size
        uint16_t                GetCountGlyph(ImFontAtlas* atlas);
        const FT_Glyph_Metrics* LoadGlyph(uint32_t in_codepoint);
        const FT_Bitmap* RenderGlyphAndGetInfo(GlyphInfo* out_glyph_info);
        void                    BlitGlyph(const FT_Bitmap* ft_bitmap, uint32_t* dst, uint32_t dst_pitch, unsigned char* multiply_table = NULL);
        ~FreeTypeFont() { CloseFont(); }

        // [Internals]
        FontInfo        Info;               // Font descriptor of the current font.
        FT_Face         Face;
        unsigned int    UserFlags;          // = ImFontConfig::RasterizerFlags
        FT_Int32        LoadFlags;
        FT_Render_Mode  RenderMode;
    };

    // From SDL_ttf: Handy routines for converting from fixed point
#define FT_CEIL(X)  (((X + 63) & -64) / 64)

    bool FreeTypeFont::InitFont(FT_Library ft_library, const ImFontConfig& cfg, unsigned int extra_user_flags, FT_Error* vFT_Error)
    {
        ZoneScoped;

        FT_Error error = FT_New_Memory_Face(ft_library, (uint8_t*)cfg.FontData, (uint32_t)cfg.FontDataSize, (uint32_t)cfg.FontNo, &Face);
        if (vFT_Error)
            *vFT_Error = error;
        if (error != 0)
            return false;
        error = FT_Select_Charmap(Face, FT_ENCODING_UNICODE);
        if (vFT_Error)
            *vFT_Error = error;
        if (error != 0)
            return false;

        memset(&Info, 0, sizeof(Info));
        SetPixelHeight((uint32_t)cfg.SizePixels);

        // Convert to FreeType flags (NB: Bold and Oblique are processed separately)
        UserFlags = cfg.RasterizerFlags | extra_user_flags;
        LoadFlags = FT_LOAD_NO_BITMAP;
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_NoHinting)
            LoadFlags |= FT_LOAD_NO_HINTING;
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_NoAutoHint)
            LoadFlags |= FT_LOAD_NO_AUTOHINT;
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_ForceAutoHint)
            LoadFlags |= FT_LOAD_FORCE_AUTOHINT;
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_LightHinting)
            LoadFlags |= FT_LOAD_TARGET_LIGHT;
        else if (UserFlags & ImGuiFreeType_unleashed::FreeType_MonoHinting)
            LoadFlags |= FT_LOAD_TARGET_MONO;
        else
            LoadFlags |= FT_LOAD_TARGET_NORMAL;

        if (UserFlags & ImGuiFreeType_unleashed::FreeType_Monochrome)
            RenderMode = FT_RENDER_MODE_MONO;
        else
            RenderMode = FT_RENDER_MODE_NORMAL;

        if (UserFlags & ImGuiFreeType_unleashed::FreeType_LoadColor)
            LoadFlags |= FT_LOAD_COLOR;

        return true;
    }

    void FreeTypeFont::CloseFont()
    {
        ZoneScoped;

        if (Face)
        {
            FT_Done_Face(Face);
            Face = NULL;
        }
    }

    void FreeTypeFont::SetPixelHeight(int pixel_height)
    {
        ZoneScoped;

        // Vuhdo: I'm not sure how to deal with font sizes properly. As far as I understand, currently ImGui assumes that the 'pixel_height'
        // is a maximum height of an any given glyph, i.e. it's the sum of font's ascender and descender. Seems strange to me.
        // NB: FT_Set_Pixel_Sizes() doesn't seem to get us the same result.
        FT_Size_RequestRec req;
        req.type = FT_SIZE_REQUEST_TYPE_REAL_DIM;
        req.width = 0;
        req.height = (uint32_t)pixel_height * 64;
        req.horiResolution = 0;
        req.vertResolution = 0;
        FT_Request_Size(Face, &req);

        // Update font info
        FT_Size_Metrics metrics = Face->size->metrics;
        Info.PixelHeight = (uint32_t)pixel_height;
        Info.Ascender = (float)FT_CEIL(metrics.ascender);
        Info.Descender = (float)FT_CEIL(metrics.descender);
        Info.LineSpacing = (float)FT_CEIL(metrics.height);
        Info.LineGap = (float)FT_CEIL(metrics.height - metrics.ascender + metrics.descender);
        Info.MaxAdvanceWidth = (float)FT_CEIL(metrics.max_advance);
    }

    inline uint16_t getUSHORT(uint8_t* p) { return p[0] * 256 + p[1]; }
    inline uint32_t getULONG(uint8_t* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }

    uint16_t FreeTypeFont::GetCountGlyph(ImFontAtlas* atlas)
    {
        ZoneScoped;

        if (atlas)
        {
            stbtt_fontinfo fontInfo;
            const int font_offset = stbtt_GetFontOffsetForIndex(
                (unsigned char*)atlas->ConfigData[0].FontData,
                atlas->ConfigData[0].FontNo);
            if (!stbtt_InitFont(&fontInfo,
                (unsigned char*)atlas->ConfigData[0].FontData, font_offset))
                return 0U;

#define gettag4(p,c0,c1,c2,c3) ((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define gettag(p,str)           gettag4(p,str[0],str[1],str[2],str[3])

            // get table offet and length
            // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html => Table Directory
            int32_t num_tables = getUSHORT(fontInfo.data + fontInfo.fontstart + 4);
            uint32_t tabledir = fontInfo.fontstart + 12;
            uint32_t tablePos = 0;
            uint32_t tableLen = 0;
            for (int i = 0; i < num_tables; ++i)
            {
                uint32_t loc = tabledir + 16 * i;
                if (gettag(fontInfo.data + loc + 0, "maxp"))
                {
                    tablePos = getULONG(fontInfo.data + loc + 8);
                    tableLen = getULONG(fontInfo.data + loc + 12);
                    break;
                }
            }
            if (!tablePos) return 0U;
#undef gettag4
#undef gettag
            // fill map of names
            uint8_t* data = fontInfo.data + tablePos;

            uint16_t countGlyphIndexs = (uint16_t)getUSHORT(data + 4);

            return countGlyphIndexs;
        }

        return 0U;
    }

    const FT_Glyph_Metrics* FreeTypeFont::LoadGlyph(uint32_t glyph_index)
    {
        ZoneScoped;

        FT_Error error = FT_Load_Glyph(Face, glyph_index, LoadFlags);
        if (error)
            return NULL;

        // Need an outline for this to work
        FT_GlyphSlot slot = Face->glyph;
        IM_ASSERT(slot->format == FT_GLYPH_FORMAT_OUTLINE);

        // Apply convenience transform (this is not picking from real "Bold"/"Italic" fonts! Merely applying FreeType helper transform. Oblique == Slanting)
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_Bold)
            FT_GlyphSlot_Embolden(slot);
        if (UserFlags & ImGuiFreeType_unleashed::FreeType_Oblique)
        {
            FT_GlyphSlot_Oblique(slot);
            //FT_BBox bbox;
            //FT_Outline_Get_BBox(&slot->outline, &bbox);
            //slot->metrics.width = bbox.xMax - bbox.xMin;
            //slot->metrics.height = bbox.yMax - bbox.yMin;
        }

        return &slot->metrics;
    }

    const FT_Bitmap* FreeTypeFont::RenderGlyphAndGetInfo(GlyphInfo* out_glyph_info)
    {
        ZoneScoped;

        FT_GlyphSlot slot = Face->glyph;
        FT_Error error = FT_Render_Glyph(slot, RenderMode);
        if (error != 0)
            return NULL;

        FT_Bitmap* ft_bitmap = &Face->glyph->bitmap;
        out_glyph_info->Width = (int)ft_bitmap->width;
        out_glyph_info->Height = (int)ft_bitmap->rows;
        out_glyph_info->OffsetX = Face->glyph->bitmap_left;
        out_glyph_info->OffsetY = -Face->glyph->bitmap_top;
        out_glyph_info->AdvanceX = (float)FT_CEIL(slot->advance.x);
        if (ft_bitmap->pixel_mode == FT_PIXEL_MODE_BGRA)
            out_glyph_info->colored = true;

        return ft_bitmap;
    }

    void FreeTypeFont::BlitGlyph(const FT_Bitmap* ft_bitmap, uint32_t* dst, uint32_t dst_pitch, unsigned char* multiply_table)
    {
        ZoneScoped;

        IM_ASSERT(ft_bitmap != NULL);
        const uint32_t w = ft_bitmap->width;
        const uint32_t h = ft_bitmap->rows;
        const uint8_t* src = ft_bitmap->buffer;
        const uint32_t src_pitch = ft_bitmap->pitch;

        switch (ft_bitmap->pixel_mode)
        {
        case FT_PIXEL_MODE_GRAY: // Grayscale image, 1 byte per pixel.
        {
            if (multiply_table == NULL)
            {
                for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                {
                    for (uint32_t x = 0; x < w; x++)
                        dst[x] = IM_COL32(255, 255, 255, src[x]);
                }
            }
            else
            {
                for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                {
                    for (uint32_t x = 0; x < w; x++)
                        dst[x] = IM_COL32(255, 255, 255, multiply_table[src[x]]);
                }
            }
            break;
        }
        case FT_PIXEL_MODE_MONO: // Monochrome image, 1 bit per pixel. The bits in each byte are ordered from MSB to LSB.
        {
            uint8_t color0 = multiply_table ? multiply_table[0] : 0;
            uint8_t color1 = multiply_table ? multiply_table[255] : 255;
            for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
            {
                uint8_t bits = 0;
                const uint8_t* bits_ptr = src;
                for (uint32_t x = 0; x < w; x++, bits <<= 1)
                {
                    if ((x & 7) == 0)
                        bits = *bits_ptr++;
                    dst[x] = IM_COL32(255, 255, 255, (bits & 0x80) ? color1 : color0);
                }
            }
            break;
        }
        case FT_PIXEL_MODE_BGRA:
        {
#define DE_MULTIPLY(color, alpha) (ImU32)(255.0f * (float)color / (float)alpha + 0.5f)

            if (multiply_table == NULL)
            {
                for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                {
                    for (uint32_t x = 0; x < w; x++)
                        dst[x] = IM_COL32(
                            DE_MULTIPLY(src[x * 4 + 2], src[x * 4 + 3]),
                            DE_MULTIPLY(src[x * 4 + 1], src[x * 4 + 3]),
                            DE_MULTIPLY(src[x * 4], src[x * 4 + 3]),
                            src[x * 4 + 3]);
                }
            }
            else
            {
                for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                {
                    for (uint32_t x = 0; x < w; x++)
                        dst[x] = IM_COL32(
                            multiply_table[DE_MULTIPLY(src[x * 4 + 2], src[x * 4 + 3])],
                            multiply_table[DE_MULTIPLY(src[x * 4 + 1], src[x * 4 + 3])],
                            multiply_table[DE_MULTIPLY(src[x * 4], src[x * 4 + 3])],
                            multiply_table[src[x * 4 + 3]]);
                }
            }

#undef DE_MULTIPLY
            break;
        }
        default:
            IM_ASSERT(0 && "FreeTypeFont::BlitGlyph(): Unknown bitmap pixel mode!");
        }
    }
}

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_ASSERT(x)     do { IM_ASSERT(x); } while (0)
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

struct ImFontBuildSrcGlyphFT
{
    GlyphInfo           Info;
    uint32_t            GlyphIndex;
    unsigned int* BitmapData;         // Point within one of the dst_tmp_bitmap_buffers[] array
};

struct ImFontBuildSrcDataFT
{
    FreeTypeFont        Font;
    stbrp_rect* Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    const ImWchar* SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBitVector         GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<ImFontBuildSrcGlyphFT>   GlyphsList;
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstDataFT
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBitVector         GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
// EXTRACTED FROM IMDRAW AND MODIFIED FOR FREETYPE COLR SUPPORT
// see PR : https://github.com/ocornut/imgui/pull/3369
//////////////////////////////////////////////////////////////////////////////////////////////////////

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The 2x2 white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
const int FT_FONT_ATLAS_DEFAULT_TEX_DATA_W = 108; // Actual texture will be 2 times that + 1 spacing.
const int FT_FONT_ATLAS_DEFAULT_TEX_DATA_H = 27;
static const char FT_FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FT_FONT_ATLAS_DEFAULT_TEX_DATA_W * FT_FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static void ImFreeTypeUnleashedFontAtlasBuildRender32bppRectFromString(ImFontAtlas* atlas, int x, int y, int w, int h, const char* in_str, char in_marker_char, unsigned int in_marker_pixel_value)
{
    ZoneScoped;

    IM_ASSERT(x >= 0 && x + w <= atlas->TexWidth);
    IM_ASSERT(y >= 0 && y + h <= atlas->TexHeight);
    unsigned int* out_pixel = atlas->TexPixelsRGBA32 + x + (y * atlas->TexWidth);
    for (int off_y = 0; off_y < h; off_y++, out_pixel += atlas->TexWidth, in_str += w)
        for (int off_x = 0; off_x < w; off_x++)
            out_pixel[off_x] = (in_str[off_x] == in_marker_char) ? in_marker_pixel_value : IM_COL32_BLACK_TRANS;
}

static void ImFreeTypeUnleashedFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    ZoneScoped;

    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdMouseCursors);
    IM_ASSERT(r->IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r->Width == FT_FONT_ATLAS_DEFAULT_TEX_DATA_W * 2 + 1 && r->Height == FT_FONT_ATLAS_DEFAULT_TEX_DATA_H);
        const int x_for_white = r->X;
        const int x_for_black = r->X + FT_FONT_ATLAS_DEFAULT_TEX_DATA_W + 1;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            ImFontAtlasBuildRender1bppRectFromString(atlas, x_for_white, r->Y, FT_FONT_ATLAS_DEFAULT_TEX_DATA_W, FT_FONT_ATLAS_DEFAULT_TEX_DATA_H, FT_FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', 0xFF);
            ImFontAtlasBuildRender1bppRectFromString(atlas, x_for_black, r->Y, FT_FONT_ATLAS_DEFAULT_TEX_DATA_W, FT_FONT_ATLAS_DEFAULT_TEX_DATA_H, FT_FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', 0xFF);
        }
        else
        {
            ImFreeTypeUnleashedFontAtlasBuildRender32bppRectFromString(atlas, x_for_white, r->Y, FT_FONT_ATLAS_DEFAULT_TEX_DATA_W, FT_FONT_ATLAS_DEFAULT_TEX_DATA_H, FT_FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, '.', IM_COL32_WHITE);
            ImFreeTypeUnleashedFontAtlasBuildRender32bppRectFromString(atlas, x_for_black, r->Y, FT_FONT_ATLAS_DEFAULT_TEX_DATA_W, FT_FONT_ATLAS_DEFAULT_TEX_DATA_H, FT_FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS, 'X', IM_COL32_WHITE);
        }
    }
    else
    {
        // Render 4 white pixels
        IM_ASSERT(r->Width == 2 && r->Height == 2);
        const int offset = (int)r->X + (int)r->Y * w;
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
        }
        else
        {
            atlas->TexPixelsRGBA32[offset] = atlas->TexPixelsRGBA32[offset + 1] = atlas->TexPixelsRGBA32[offset + w] = atlas->TexPixelsRGBA32[offset + w + 1] = IM_COL32_WHITE;
        }
    }
    atlas->TexUvWhitePixel = ImVec2((r->X + 0.5f) * atlas->TexUvScale.x, (r->Y + 0.5f) * atlas->TexUvScale.y);
}

static void ImFreeTypeUnleashedFontAtlasBuildRenderLinesTexData(ImFontAtlas* atlas)
{
    ZoneScoped;

    if (atlas->Flags & ImFontAtlasFlags_NoBakedLines)
        return;

    // This generates a triangular shape in the texture, with the various line widths stacked on top of each other to allow interpolation between them
    ImFontAtlasCustomRect* r = atlas->GetCustomRectByIndex(atlas->PackIdLines);
    IM_ASSERT(r->IsPacked());
    for (unsigned int n = 0; n < IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1; n++) // +1 because of the zero-width row
    {
        // Each line consists of at least two empty pixels at the ends, with a line of solid pixels in the middle
        unsigned int y = n;
        unsigned int line_width = n;
        unsigned int pad_left = (r->Width - line_width) / 2;
        unsigned int pad_right = r->Width - (pad_left + line_width);

        // Write each slice
        IM_ASSERT(pad_left + line_width + pad_right == r->Width && y < r->Height); // Make sure we're inside the texture bounds before we start writing pixels
        if (atlas->TexPixelsAlpha8 != NULL)
        {
            unsigned char* write_ptr = &atlas->TexPixelsAlpha8[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = 0x00;

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = 0xFF;

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = 0x00;
        }
        else
        {
            unsigned int* write_ptr = &atlas->TexPixelsRGBA32[r->X + ((r->Y + y) * atlas->TexWidth)];
            for (unsigned int i = 0; i < pad_left; i++)
                *(write_ptr + i) = IM_COL32_BLACK_TRANS;

            for (unsigned int i = 0; i < line_width; i++)
                *(write_ptr + pad_left + i) = IM_COL32_WHITE;

            for (unsigned int i = 0; i < pad_right; i++)
                *(write_ptr + pad_left + line_width + i) = IM_COL32_BLACK_TRANS;
        }

        // Calculate UVs for this line
        ImVec2 uv0 = ImVec2((float)(r->X + pad_left - 1), (float)(r->Y + y)) * atlas->TexUvScale;
        ImVec2 uv1 = ImVec2((float)(r->X + pad_left + line_width + 1), (float)(r->Y + y + 1)) * atlas->TexUvScale;
        float half_v = (uv0.y + uv1.y) * 0.5f; // Calculate a constant V in the middle of the row to avoid sampling artifacts
        atlas->TexUvLines[n] = ImVec4(uv0.x, half_v, uv1.x, half_v);
    }
}

// This is called/shared by both the stb_truetype and the FreeType builder.
void ImFreeTypeUnleashedFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    ZoneScoped;

    // Render into our custom data blocks
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL || atlas->TexPixelsRGBA32 != NULL);
    ImFreeTypeUnleashedFontAtlasBuildRenderDefaultTexData(atlas);
    ImFreeTypeUnleashedFontAtlasBuildRenderLinesTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlasCustomRect* r = &atlas->CustomRects[i];
        if (r->Font == NULL/* || r->GlyphID == 0*/)
            continue;

        IM_ASSERT(r->Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(r, &uv0, &uv1);
        r->Font->AddGlyph(NULL, (ImWchar)r->GlyphID, r->GlyphOffset.x, r->GlyphOffset.y, r->GlyphOffset.x + r->Width, r->GlyphOffset.y + r->Height, uv0.x, uv0.y, uv1.x, uv1.y, r->GlyphAdvanceX, r->IgnoreTint);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();

    // Ellipsis character is required for rendering elided text. We prefer using U+2026 (horizontal ellipsis).
    // However some old fonts may contain ellipsis at U+0085. Here we auto-detect most suitable ellipsis character.
    // FIXME: Also note that 0x2026 is currently seldom included in our font ranges. Because of this we are more likely to use three individual dots.
    for (int i = 0; i < atlas->Fonts.size(); i++)
    {
        ImFont* font = atlas->Fonts[i];
        if (font->EllipsisChar != (ImWchar)-1)
            continue;
        const ImWchar ellipsis_variants[] = { (ImWchar)0x2026, (ImWchar)0x0085 };
        for (int j = 0; j < IM_ARRAYSIZE(ellipsis_variants); j++)
            if (font->FindGlyphNoFallback(ellipsis_variants[j]) != NULL) // Verify glyph exists
            {
                font->EllipsisChar = ellipsis_variants[j];
                break;
            }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool ImFontAtlasBuildWithFreeTypeUnleashed(FT_Library ft_library, ImFontAtlas* atlas, unsigned int extra_flags, FT_Error* vFT_Error)
{
    ZoneScoped;

    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildInit(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcDataFT> src_tmp_array;
    ImVector<ImFontBuildDstDataFT> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    {
        ZoneScopedN("1. Initialize font loading structure");

        for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            ImFontConfig& cfg = atlas->ConfigData[src_i];
            FreeTypeFont& font_face = src_tmp.Font;
            IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

            // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
            src_tmp.DstIndex = -1;
            for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
                if (cfg.DstFont == atlas->Fonts[output_i])
                    src_tmp.DstIndex = output_i;
            IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
            if (src_tmp.DstIndex == -1)
                return false;

            // Load font
            if (!font_face.InitFont(ft_library, cfg, extra_flags, vFT_Error))
                return false;

            // Measure highest codepoints
            ImFontBuildDstDataFT& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
            static const ImWchar ranges[] =
            {
                0x0000, 0x00FF, // Basic Latin + Latin Supplement
                0,
            };
            src_tmp.SrcRanges = ranges;
            src_tmp.GlyphsHighest = 0xFFFF;
            dst_tmp.SrcCount++;
            dst_tmp.GlyphsHighest = 0xFFFF;
        }
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    {
        ZoneScopedN("2. check for their presence in the font data");

        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            ImFontBuildDstDataFT& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
            src_tmp.GlyphsSet.Create(src_tmp.GlyphsHighest + 1);
            if (dst_tmp.GlyphsSet.Storage.empty())
                dst_tmp.GlyphsSet.Create(dst_tmp.GlyphsHighest + 1);

            for (int glyph_index = 0x0000; glyph_index <= 0xFFFF; glyph_index++)
            {
                if (dst_tmp.GlyphsSet.TestBit(glyph_index))    // Don't overwrite existing glyphs. We could make this an option (e.g. MergeOverwrite)
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(glyph_index);
                dst_tmp.GlyphsSet.SetBit(glyph_index);
                total_glyphs_count++;
            }
        }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    {
        ZoneScopedN("3. Unpack our bit map into a flat list");

        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);

            IM_ASSERT(sizeof(src_tmp.GlyphsSet.Storage.Data[0]) == sizeof(ImU32));
            const ImU32* it_begin = src_tmp.GlyphsSet.Storage.begin();
            const ImU32* it_end = src_tmp.GlyphsSet.Storage.end();
            for (const ImU32* it = it_begin; it < it_end; it++)
                if (ImU32 entries_32 = *it)
                    for (ImU32 bit_n = 0; bit_n < 32; bit_n++)
                        if (entries_32 & ((ImU32)1 << bit_n))
                        {
                            ImFontBuildSrcGlyphFT src_glyph;
                            memset(&src_glyph, 0, sizeof(src_glyph));
                            src_glyph.GlyphIndex = (ImWchar)(((it - it_begin) << 5) + bit_n);
                            //src_glyph.GlyphIndex = 0; // FIXME-OPT: We had this info in the previous step and lost it..
                            src_tmp.GlyphsList.push_back(src_glyph);
                        }
            src_tmp.GlyphsSet.Clear();
            IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
        }
        for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
            dst_tmp_array[dst_i].GlyphsSet.Clear();
        dst_tmp_array.clear();
    }

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    buf_rects.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());

    // Allocate temporary rasterization data buffers.
    // We could not find a way to retrieve accurate glyph size without rendering them.
    // (e.g. slot->metrics->width not always matching bitmap->width, especially considering the Oblique transform)
    // We allocate in chunks of 256 KB to not waste too much extra memory ahead. Hopefully users of FreeType won't find the temporary allocations.
    const int BITMAP_BUFFERS_CHUNK_SIZE = 256 * 1024;
    int buf_bitmap_current_used_bytes = 0;
    ImVector<unsigned char*> buf_bitmap_buffers;
    buf_bitmap_buffers.push_back((unsigned char*)IM_ALLOC(BITMAP_BUFFERS_CHUNK_SIZE));

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    // 5. Render/rasterize font characters into the texture
    int total_surface = 0;
    int buf_rects_out_n = 0;
    {
        ZoneScopedN("4. Gather glyphs sizes then 5. Render/rasterize font characters into the texture");

        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            ImFontConfig& cfg = atlas->ConfigData[src_i];
            if (src_tmp.GlyphsCount == 0)
                continue;

            src_tmp.Rects = &buf_rects[buf_rects_out_n];
            buf_rects_out_n += src_tmp.GlyphsCount;

            // Compute multiply table if requested
            const bool multiply_enabled = (cfg.RasterizerMultiply != 1.0f);
            unsigned char multiply_table[256];
            if (multiply_enabled)
                ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);

            uint16_t countGlyphIndex = src_tmp.Font.GetCountGlyph(atlas);

            // Gather the sizes of all rectangles we will need to pack
            const int padding = atlas->TexGlyphPadding;
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
            {
                ImFontBuildSrcGlyphFT& src_glyph = src_tmp.GlyphsList[glyph_i];

                if (src_glyph.GlyphIndex >= countGlyphIndex)
                    continue;

                const FT_Glyph_Metrics* metrics = src_tmp.Font.LoadGlyph(src_glyph.GlyphIndex);
                if (metrics == NULL)
                    continue;

                // Render glyph into a bitmap (currently held by FreeType)
                const FT_Bitmap* ft_bitmap = src_tmp.Font.RenderGlyphAndGetInfo(&src_glyph.Info);
                IM_ASSERT(ft_bitmap);

                // Allocate new temporary chunk if needed
                const int bitmap_size_in_bytes = src_glyph.Info.Width * src_glyph.Info.Height * 4;
                if (buf_bitmap_current_used_bytes + bitmap_size_in_bytes > BITMAP_BUFFERS_CHUNK_SIZE)
                {
                    buf_bitmap_current_used_bytes = 0;
                    buf_bitmap_buffers.push_back((unsigned char*)IM_ALLOC(BITMAP_BUFFERS_CHUNK_SIZE));
                }

                // Blit rasterized pixels to our temporary buffer and keep a pointer to it.
                src_glyph.BitmapData = (unsigned int*)(buf_bitmap_buffers.back() + buf_bitmap_current_used_bytes);
                buf_bitmap_current_used_bytes += bitmap_size_in_bytes;
                src_tmp.Font.BlitGlyph(ft_bitmap, src_glyph.BitmapData, src_glyph.Info.Width, multiply_enabled ? multiply_table : NULL);

                src_tmp.Rects[glyph_i].w = (stbrp_coord)(src_glyph.Info.Width + padding);
                src_tmp.Rects[glyph_i].h = (stbrp_coord)(src_glyph.Info.Height + padding);
                total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
            }
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096 * 0.7f) ? 4096 : (surface_sqrt >= 2048 * 0.7f) ? 2048 : (surface_sqrt >= 1024 * 0.7f) ? 1024 : 512;

    // 6. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    stbrp_context pack_context;
    ImVector<stbrp_node> pack_nodes;
    const int TEX_HEIGHT_MAX = 1024 * 32;
    const int num_nodes_for_packing_algorithm = atlas->TexWidth - atlas->TexGlyphPadding;
    {
        ZoneScopedN("6. Start packing");

        pack_nodes.resize(num_nodes_for_packing_algorithm);
        stbrp_init_target(&pack_context, atlas->TexWidth, TEX_HEIGHT_MAX, pack_nodes.Data, pack_nodes.Size);
        ImFontAtlasBuildPackCustomRects(atlas, &pack_context);
    }

    // 7. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    {
        ZoneScopedN("7. Pack each source font");

        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            if (src_tmp.GlyphsCount == 0)
                continue;

            stbrp_pack_rects(&pack_context, src_tmp.Rects, src_tmp.GlyphsCount);

            // Extend texture height and mark missing glyphs as non-packed so we won't render them.
            // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
                if (src_tmp.Rects[glyph_i].was_packed)
                    atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
        }
    }

    // 8. Allocate texture
    {
        ZoneScopedN("8. Allocate texture");

        atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
        atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
        if ((extra_flags & ImGuiFreeType_unleashed::FreeType_LoadColor) == ImGuiFreeType_unleashed::FreeType_LoadColor)
        {
            atlas->TexPixelsRGBA32 = (unsigned int*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight * 4);
            memset(atlas->TexPixelsRGBA32, 0, atlas->TexWidth * atlas->TexHeight * 4);
        }
        else
        {
            atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
            memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
        }
    }

    // 9. Copy rasterized font characters back into the main texture
    // 10. Setup ImFont and glyphs for runtime
    {
        ZoneScopedN("9. Allocate texture then 10. Setup ImFont and glyphs for runtime");

        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        {
            ImFontBuildSrcDataFT& src_tmp = src_tmp_array[src_i];
            if (src_tmp.GlyphsCount == 0)
                continue;

            ImFontConfig& cfg = atlas->ConfigData[src_i];
            ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

            const float ascent = src_tmp.Font.Info.Ascender;
            const float descent = src_tmp.Font.Info.Descender;
            ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
            const float font_off_x = cfg.GlyphOffset.x;
            const float font_off_y = cfg.GlyphOffset.y + IM_ROUND(dst_font->Ascent);

            const int padding = atlas->TexGlyphPadding;
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            {
                ImFontBuildSrcGlyphFT& src_glyph = src_tmp.GlyphsList[glyph_i];
                stbrp_rect& pack_rect = src_tmp.Rects[glyph_i];
                IM_ASSERT(pack_rect.was_packed);
                if (pack_rect.w == 0 && pack_rect.h == 0)
                    continue;

                GlyphInfo& info = src_glyph.Info;
                IM_ASSERT(info.Width + padding <= pack_rect.w);
                IM_ASSERT(info.Height + padding <= pack_rect.h);
                const int tx = pack_rect.x + padding;
                const int ty = pack_rect.y + padding;

                // Blit from temporary buffer to final texture
                size_t blit_src_stride = (size_t)src_glyph.Info.Width;
                size_t blit_dst_stride = (size_t)atlas->TexWidth;
                unsigned int* blit_src = src_glyph.BitmapData;
                if (atlas->TexPixelsAlpha8 != NULL)
                {
                    unsigned char* blit_dst = atlas->TexPixelsAlpha8 + (ty * blit_dst_stride) + tx;
                    for (int y = 0; y < info.Height; y++, blit_dst += blit_dst_stride, blit_src += blit_src_stride)
                        for (int x = 0; x < info.Width; x++)
                            blit_dst[x] = (unsigned char)((blit_src[x] >> IM_COL32_A_SHIFT) & 0xFF);
                }
                else
                {
                    unsigned int* blit_dst = atlas->TexPixelsRGBA32 + (ty * blit_dst_stride) + tx;
                    for (int y = 0; y < info.Height; y++, blit_dst += blit_dst_stride, blit_src += blit_src_stride)
                        for (int x = 0; x < info.Width; x++)
                            blit_dst[x] = blit_src[x];
                }

                float char_advance_x_org = info.AdvanceX;
                float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
                float char_off_x = font_off_x;
                if (char_advance_x_org != char_advance_x_mod)
                    char_off_x += cfg.PixelSnapH ? IM_FLOOR((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

                // Register glyph
                float x0 = info.OffsetX + char_off_x;
                float y0 = info.OffsetY + font_off_y;
                float x1 = x0 + info.Width;
                float y1 = y0 + info.Height;
                float u0 = (tx) / (float)atlas->TexWidth;
                float v0 = (ty) / (float)atlas->TexHeight;
                float u1 = (tx + info.Width) / (float)atlas->TexWidth;
                float v1 = (ty + info.Height) / (float)atlas->TexHeight;
                dst_font->AddGlyph(&cfg, (ImWchar)src_glyph.GlyphIndex, x0, y0, x1, y1, u0, v0, u1, v1, char_advance_x_mod, info.colored);
            }

            src_tmp.Rects = NULL;
        }
    }

    // Cleanup
    {
        ZoneScopedN("11. Cleanup");

        for (int buf_i = 0; buf_i < buf_bitmap_buffers.Size; buf_i++)
            IM_FREE(buf_bitmap_buffers[buf_i]);
        for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
            src_tmp_array[src_i].~ImFontBuildSrcDataFT();
    }

    ImFreeTypeUnleashedFontAtlasBuildFinish(atlas);

    return true;
}

// Default memory allocators
static void* ImFreeTypeDefaultAllocFunc(size_t size, void* user_data) { IM_UNUSED(user_data); return IM_ALLOC(size); }
static void  ImFreeTypeDefaultFreeFunc(void* ptr, void* user_data) { IM_UNUSED(user_data); IM_FREE(ptr); }

// Current memory allocators
static void* (*GImFreeTypeAllocFunc)(size_t size, void* user_data) = ImFreeTypeDefaultAllocFunc;
static void  (*GImFreeTypeFreeFunc)(void* ptr, void* user_data) = ImFreeTypeDefaultFreeFunc;
static void* GImFreeTypeAllocatorUserData = NULL;

// FreeType memory allocation callbacks
static void* FreeType_Alloc(FT_Memory /*memory*/, long size)
{
    ZoneScoped;

    return GImFreeTypeAllocFunc((size_t)size, GImFreeTypeAllocatorUserData);
}

static void FreeType_Free(FT_Memory /*memory*/, void* block)
{
    ZoneScoped;

    GImFreeTypeFreeFunc(block, GImFreeTypeAllocatorUserData);
}

static void* FreeType_Realloc(FT_Memory /*memory*/, long cur_size, long new_size, void* block)
{
    ZoneScoped;

    // Implement realloc() as we don't ask user to provide it.
    if (block == NULL)
        return GImFreeTypeAllocFunc((size_t)new_size, GImFreeTypeAllocatorUserData);

    if (new_size == 0)
    {
        GImFreeTypeFreeFunc(block, GImFreeTypeAllocatorUserData);
        return NULL;
    }

    if (new_size > cur_size)
    {
        void* new_block = GImFreeTypeAllocFunc((size_t)new_size, GImFreeTypeAllocatorUserData);
        memcpy(new_block, block, (size_t)cur_size);
        GImFreeTypeFreeFunc(block, GImFreeTypeAllocatorUserData);
        return new_block;
    }

    return block;
}

const char* ImGuiFreeType_unleashed::GetErrorMessage(FT_Error err)
{
    ZoneScoped;

#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
    return "(Unknown error)";
}

bool ImGuiFreeType_unleashed::BuildFontAtlas(ImFontAtlas* atlas, unsigned int extra_flags, FT_Error* vFreetypeError)
{
    ZoneScoped;

    // FreeType memory management: https://www.freetype.org/freetype2/docs/design/design-4.html
    FT_MemoryRec_ memory_rec = {};
    memory_rec.user = NULL;
    memory_rec.alloc = &FreeType_Alloc;
    memory_rec.free = &FreeType_Free;
    memory_rec.realloc = &FreeType_Realloc;

    // https://www.freetype.org/freetype2/docs/reference/ft2-module_management.html#FT_New_Library
    FT_Library ft_library;
    FT_Error error = FT_New_Library(&memory_rec, &ft_library);
    if (error != 0)
        return false;

    // If you don't call FT_Add_Default_Modules() the rest of code may work, but FreeType won't use our custom allocator.
    FT_Add_Default_Modules(ft_library);

    bool ret = ImFontAtlasBuildWithFreeTypeUnleashed(ft_library, atlas, extra_flags, vFreetypeError);
    FT_Done_Library(ft_library);

    return ret;
}

void ImGuiFreeType_unleashed::SetAllocatorFunctions(void* (*alloc_func)(size_t sz, void* user_data), void (*free_func)(void* ptr, void* user_data), void* user_data)
{
    ZoneScoped;

    GImFreeTypeAllocFunc = alloc_func;
    GImFreeTypeFreeFunc = free_func;
    GImFreeTypeAllocatorUserData = user_data;
}

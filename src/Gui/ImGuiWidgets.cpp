// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ImGuiWidgets.h"
#include <FileHelper.h>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

bool ImGui::SelectableWithBtn(
        const char* label,
        bool selected,
        const char* btnLabel,
        bool *btnClicked,
        bool *btnHovered,
        ImVec4 vBtnColor,
        ImVec4 vBtnHoveredColor,
        ImGuiSelectableFlags flags,
        const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns) // FIXME-OPT: Avoid if vertically clipped.
        PushColumnsBackground();

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, nullptr, true);
    ImVec2 btn_label_size = btnLabel ? CalcTextSize(btnLabel, nullptr, true) : ImVec2(0, 0);
    ImVec2 size(size_arg.x > 0.0f ? size_arg.x : label_size.x, size_arg.y > 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImRect bb_inner(pos, pos + size);
    ItemSize(size, 0.0f);

    // Fill horizontal space.
    ImVec2 window_padding = window->WindowPadding;
    float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
    ImVec2 size_draw((size_arg.x > 0.0f && !(flags & ImGuiSelectableFlags_SpanAvailWidth)) ? size_arg.x : w_draw, size_arg.y > 0.0f ? size_arg.y : size.y);
    size_draw.x -= btn_label_size.x * 1.5f;
    ImRect bb(pos, pos + size_draw);
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        bb.Max.x += window_padding.x;

    // Selectables are tightly packed together so we extend the box to cover spacing between selectable.
    const float spacing_x = style.ItemSpacing.x;
    const float spacing_y = style.ItemSpacing.y;
    const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
    const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
    bb.Min.x -= spacing_L;
    bb.Min.y -= spacing_U;
    bb.Max.x += (spacing_x - spacing_L);
    bb.Max.y += (spacing_y - spacing_U);

    bool item_add;
    if (flags & ImGuiSelectableFlags_Disabled)
    {
        ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
        window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        item_add = ItemAdd(bb, id);
        window->DC.ItemFlags = backup_item_flags;
    }
    else
    {
        item_add = ItemAdd(bb, id);
    }
    if (!item_add)
    {
        if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
            PopColumnsBackground();
        return false;
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    //if (flags & ImGuiSelectableFlags_PressedOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    //if (flags & ImGuiSelectableFlags_PressedOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_Disabled) { button_flags |= ImGuiButtonFlags_Disabled; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if (flags & ImGuiSelectableFlags_AllowItemOverlap) { button_flags |= ImGuiButtonFlags_AllowItemOverlap; }

    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            g.NavDisableHighlight = true;
            SetNavID(id, window->DC.NavLayerCurrent, window->DC.NavFocusScopeIdCurrent);
        }
    }
    if (pressed)
        MarkItemEdited(id);

    if (flags & ImGuiSelectableFlags_AllowItemOverlap)
        SetItemAllowOverlap();

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
        hovered = true;
    if (hovered || selected)
    {
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
        RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (btnLabel && btnClicked)
    {
        // item
        ImGuiID extraId = window->GetID((void*)(intptr_t)(id + 1));
        ImVec2 p = ImVec2(bb.Max.x, bb.Min.y + btn_label_size.y * 0.33f);
        ImRect bbMenu(p, p + btn_label_size);

        // detection
        bool bh, he;
        *btnClicked = ImGui::ButtonBehavior(bbMenu, extraId, &bh, &he, button_flags);

        if (btnHovered)
            *btnHovered = bh;

        // render
        const ImU32 col = ImGui::GetColorU32(he || bh ? vBtnHoveredColor : vBtnColor);
        window->DrawList->AddText(bbMenu.Min, col, btnLabel);
    }

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
    {
        PopColumnsBackground();
        bb.Max.x -= (GetContentRegionMax().x - max_x);
    }

    if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
    RenderTextClipped(bb_inner.Min, bb_inner.Max, label, nullptr, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    return pressed;
}

#define ImRatioX(a) a.x / a.y
#define ImRatioY(a) a.y / a.x

// based on ImGui::ImageButton
bool ImGui::ImageCheckButton(
        ImTextureID user_texture_id, bool *v,
        const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1,
        const ImVec2& vHostTextureSize, int frame_padding,
        float vRectThickNess, ImVec4 vRectColor)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
    PushID((void*)(intptr_t)user_texture_id);
    const ImGuiID id = window->GetID("#image");
    PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
    const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    if (pressed && v)
        *v = !*v;

    // Render
    const ImU32 col = GetColorU32(((held && hovered) || (v && *v)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (vRectThickNess > 0.0f)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vRectColor), 0.0, 15, vRectThickNess);
    }

    // resize with respect to glyph ratio
    float hostRatioX = 1.0f;
    if (vHostTextureSize.y > 0)
        hostRatioX = ImRatioX(vHostTextureSize);
    float ratioX = ImRatioX(ImVec2(uv1 - uv0)) * hostRatioX;
    ImVec2 imgSize = image_bb.GetSize();
    float newX = imgSize.y * ratioX;
    ImVec2 glyphSize = ImVec2(imgSize.x, imgSize.x / ratioX) * 0.5f;
    if (newX < imgSize.x) glyphSize = ImVec2(newX, imgSize.y) * 0.5f;
    ImVec2 center = image_bb.GetCenter();
    window->DrawList->AddImage(user_texture_id, center - glyphSize, center + glyphSize, uv0, uv1, GetColorU32(ImGuiCol_Text));

    return pressed;
}

bool ImGui::BeginFramedGroup(const char *vLabel, bool vSpacing, ImVec4 /*vCol*/, ImVec4 /*vHoveredCol*/)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->ChannelsSplit(2); // split for have 2 layers

    draw_list->ChannelsSetCurrent(1); // Layer ForeGround

    if (vSpacing)
        ImGui::Spacing();

    ImGui::Spacing();
    ImGui::Indent();

    ImGui::BeginGroup();

    if (vLabel)
    {
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 1));

        //ImGuiWindow* window = ImGui::GetCurrentWindow();
        //if (!window->SkipItems)
        {
            // header
            const ImGuiID id = window->GetID(vLabel);
            const ImVec2 label_size = ImGui::CalcTextSize(vLabel, nullptr, true);

            const float frame_height =
                    ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2),
                          label_size.y + style.FramePadding.y * 2);
            ImRect frame_bb;
            frame_bb.Min.x = window->WorkRect.Min.x;
            frame_bb.Min.y = window->DC.CursorPos.y - 5.0f;
            frame_bb.Max.x = window->WorkRect.Max.x;
            frame_bb.Max.y = window->DC.CursorPos.y + frame_height;

            ImGui::ItemSize(frame_bb, style.FramePadding.y);
            if (ImGui::ItemAdd(frame_bb, id))
            {
                bool hovered, held;
                ImGui::ButtonBehavior(frame_bb, id, &hovered, &held);

                // Render
                const ImU32 frameCol = ImGui::GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
                ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frameCol, true, style.FrameRounding);
                ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, vLabel, nullptr, &label_size, style.ButtonTextAlign, &frame_bb);
            }
        }

        ImGui::PopStyleVar();
    }

    return true;
}

void ImGui::EndFramedGroup(bool vSpacing)
{
    ImGui::EndGroup();

    ImGui::Unindent();
    ImGui::Spacing();

    if (vSpacing)
        ImGui::Spacing();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    draw_list->ChannelsSetCurrent(0); // Layer Background

    ImVec2 p_min = ImGui::GetItemRectMin();
    p_min.x = window->WorkRect.Min.x;
    p_min.y -= 5.0f;

    ImVec2 p_max = ImGui::GetItemRectMax();
    p_max.x = window->WorkRect.Max.x;
    p_max.y += 5.0f;

    draw_list->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGuiCol_WindowBg), style.FrameRounding);

    draw_list->ChannelsSetCurrent(1); // Layer Foreground

    const ImU32 frameCol = ImGui::GetColorU32(ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    draw_list->AddRect(p_min, p_max, frameCol, style.FrameRounding, ImDrawCornerFlags_Top, 4.0f);

    draw_list->ChannelsMerge(); // merge layers
}

bool ImGui::RadioButtonLabeled(const char* label, bool active, bool disabled)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    float w = CalcItemWidth();
    if (w == window->ItemWidthDefault)	w = 0.0f; // no push item width
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    ImVec2 bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
    bb_size.x = ImMax(w, bb_size.x);

    const ImRect check_bb(
            window->DC.CursorPos,
            window->DC.CursorPos + bb_size);
    ItemSize(check_bb, style.FramePadding.y);

    if (!ItemAdd(check_bb, id))
        return false;

    // check
	bool pressed = false;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(check_bb, id, &hovered, &held);

		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
		if (active)
		{
			const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
		}
	}
    
    // circle shadow + bg
    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (label_size.x > 0.0f)
    {
		RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
    }

    return pressed;
}

bool ImGui::RadioButtonLabeled(const char* label, const char* help, bool active, bool disabled)
{
    bool change = RadioButtonLabeled(label, active, disabled);
    if (help)
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", help);
    return change;
}

bool ImGui::RadioButtonLabeled(const char* label, const char* help, bool *active, bool disabled)
{
    bool change = RadioButtonLabeled(label, *active, disabled);
    if (change)
        *active = !*active;
    if (help)
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", help);
    return change;
}

bool ImGui::CollapsingHeader_SmallHeight(const char *vName, float vHeightRatio, float vWidth, bool vDefaulExpanded, bool *vIsOpen)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(vName);
    ImVec2 label_size = ImGui::CalcTextSize(vName, nullptr, true);
    label_size.y *= vHeightRatio;

    //label_size.x = ImMin(label_size.x, vWidth);
    ImVec2 padding = style.FramePadding;
    padding.y *= vHeightRatio;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 nsize = ImGui::CalcItemSize(
            ImVec2(vWidth, label_size.y + padding.y * 2.0f),
            label_size.x + padding.x * 2.0f, label_size.y + padding.y * 2.0f);
    //nsize.y *= vHeightRatio;

    ImRect bb(pos, pos + nsize);
    ImRect bbTrigger = bb;
    ImGui::ItemSize(bb, padding.y);
    ImGui::ItemAdd(bb, id);

    bool is_open = vDefaulExpanded;
    if (vIsOpen && !*vIsOpen) is_open = false;
    is_open = window->DC.StateStorage->GetInt(id, is_open ? 1 : 0) != 0;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bbTrigger, id, &hovered, &held, 0);
    if (pressed)
    {
        is_open = !is_open;
        window->DC.StateStorage->SetInt(id, is_open);
        if (vIsOpen)
            *vIsOpen = is_open;
    }

    // Render
    static ImVec4 _ScrollbarGrab(0.5f, 0.0f, 1.0f, 0.50f);
    static ImVec4 _ScrollbarGrabHovered(0.4f, 0.0f, 0.75f, 0.90f);
    static ImVec4 _ScrollbarGrabActive(0.3f, 0.0f, 0.5f, 0.90f);

    const ImU32 col = ImGui::GetColorU32((held && hovered) ? _ScrollbarGrabActive : hovered ? _ScrollbarGrabHovered : _ScrollbarGrab);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    ImGui::RenderTextClipped(bb.Min, bb.Max - padding, vName, nullptr, &label_size, style.ButtonTextAlign, &bb);
    padding.y *= vHeightRatio;
    RenderArrow(window->DrawList, bb.Min + padding, ImGui::GetColorU32(ImGuiCol_Text),
                (is_open ? ImGuiDir_::ImGuiDir_Down : ImGuiDir_::ImGuiDir_Right), 1.0f);

    return is_open;
}

bool ImGui::ClickableTextUrl(const char* label, const char* url, bool vOnlined)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, 0.0f);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	ImGuiButtonFlags flags = 0;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
	if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	ImGui::RenderNavHighlight(bb, id);
	ImVec4 defColor = ImGui::GetStyleColorVec4(ImGuiCol_Text); defColor.w = 0.5f;
	ImVec4 hovColor = defColor; hovColor.w = 0.75f;
	ImVec4 actColor = defColor; actColor.w = 1.0f;
	ImVec4 col = (hovered && held) ? actColor : hovered ? hovColor : defColor;
	ImVec2 p0 = bb.Min;
	ImVec2 p1 = bb.Max;
	if (hovered && held)
	{
		p0 += ImVec2(1, 1);
		p1 += ImVec2(1, 1);
	}
	if (vOnlined)
		window->DrawList->AddLine(ImVec2(p0.x + style.FramePadding.x, p1.y), ImVec2(p1.x - style.FramePadding.x, p1.y), ImGui::GetColorU32(col));
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	ImGui::RenderTextClipped(p0, p1, label, nullptr, &label_size, style.ButtonTextAlign, &bb);
	ImGui::PopStyleColor();

	if (hovered)
	{
		ImGui::SetTooltip("%s", url);
	}

	if (pressed)
	{
		FileHelper::Instance()->OpenUrl(url);
	}

	return pressed;
}

/*bool ImGui::ClickableTextUrl(const char* label, const char* url, bool vOnlined, ImVec4 vColor, ImVec4 vHoveredColor, ImVec4 vClickColor)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, 0.0f);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    ImGuiButtonFlags flags = 0;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
    if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::RenderNavHighlight(bb, id);
    ImVec4 col = (hovered && held) ? vClickColor : hovered ? vHoveredColor : vColor;
    ImVec2 p0 = bb.Min;
    ImVec2 p1 = bb.Max;
    if (hovered && held)
    {
        p0 += ImVec2(1, 1);
        p1 += ImVec2(1, 1);
    }
    if (vOnlined)
        window->DrawList->AddLine(ImVec2(p0.x + style.FramePadding.x, p1.y), ImVec2(p1.x - style.FramePadding.x, p1.y), ImGui::GetColorU32(col));
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::RenderTextClipped(p0, p1, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor();

    if (hovered)
    {
        ImGui::SetTooltip(url);
    }

    if (pressed)
    {
        FileHelper::Instance()->OpenUrl(url);
    }

    return pressed;
}

bool ImGui::ClickableTextFile(const char* label, const char* file, bool vOnlined, ImVec4 vColor, ImVec4 vHoveredColor, ImVec4 vClickColor)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, 0.0f);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    ImGuiButtonFlags flags = 0;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
    if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    ImGui::RenderNavHighlight(bb, id);
    ImVec4 col = (hovered && held) ? vClickColor : hovered ? vHoveredColor : vColor;
    ImVec2 p0 = bb.Min;
    ImVec2 p1 = bb.Max;
    if (hovered && held)
    {
        p0 += ImVec2(1, 1);
        p1 += ImVec2(1, 1);
    }
    if (vOnlined)
        window->DrawList->AddLine(ImVec2(p0.x + style.FramePadding.x, p1.y), ImVec2(p1.x - style.FramePadding.x, p1.y), ImGui::GetColorU32(col));
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::RenderTextClipped(p0, p1, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor();

    if (hovered)
    {
        ImGui::SetTooltip(file);
    }

    if (pressed)
    {
        FileHelper::Instance()->OpenFile(file);
    }

    return pressed;
}*/

void ImGui::Spacing(float vSpace)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ItemSize(ImVec2(vSpace, 0));
}

ImGuiWindow* ImGui::GetHoveredWindow()
{
	ImGuiContext& g = *GImGui;
	return g.HoveredWindow;
}

bool ImGui::BeginMainStatusBar()
{
	ImGuiContext& g = *GImGui;
	ImGuiViewportP* viewport = g.Viewports[0];

	// For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
	g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));

	// Get our rectangle in the work area, and report the size we need for next frame.
	// We don't attempt to calculate our height ahead, as it depends on the per-viewport font size. However menu-bar will affect the minimum window size so we'll get the right height.
	ImVec2 menu_bar_size = ImVec2(viewport->Size.x - viewport->CurrWorkOffsetMin.x + viewport->CurrWorkOffsetMax.x, 1.0f);
	
	// Create window
	//SetNextWindowPos(menu_bar_pos);
	SetNextWindowSize(menu_bar_size);
	SetNextWindowViewport(viewport->ID); // Enforce viewport so we don't create our own viewport when ImGuiConfigFlags_ViewportsNoMerge is set.
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));    // Lift normal size constraint, however the presence of a menu-bar will give us the minimum height we want.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	bool is_open = Begin("##MainStatusBar", nullptr, window_flags) && BeginMenuBar();
	PopStyleVar(2);

	float barH = 0.0f;
	auto curWin = ImGui::GetCurrentWindowRead();
	if (curWin)
		barH = curWin->MenuBarHeight();
	ImVec2 menu_bar_pos = viewport->Pos + viewport->CurrWorkOffsetMin + ImVec2(0.0f, viewport->Size.y - barH);
	SetWindowPos(menu_bar_pos);

	// Feed back into work area using actual window size
	viewport->CurrWorkOffsetMin.y += GetCurrentWindow()->Size.y;

	g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);
	if (!is_open)
	{
		End();
		return false;
	}
	return true; //-V1020
}

void ImGui::EndMainStatusBar()
{
	EndMenuBar();

	// When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
	// FIXME: With this strategy we won't be able to restore a NULL focus.
	ImGuiContext& g = *GImGui;
	if (g.CurrentWindow == g.NavWindow && g.NavLayer == 0 && !g.NavAnyRequest)
		FocusTopMostWindowUnderOne(g.NavWindow, nullptr);

	End();
}

bool ImGui::Button(const char* label, const char* help)
{
	bool res = ImGui::Button(label);

	if (help)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", help);

	return res;
}

bool ImGui::Selectable_FramedText(const char* fmt, ...)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	va_list args;
	va_start(args, fmt);
	const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
	va_end(args);

	ImVec2 label_size = CalcTextSize(g.TempBuffer, text_end, true);

	const ImGuiID id = window->GetID(g.TempBuffer);

	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.x + style.FramePadding.x * 4 - 1, label_size.y + style.FramePadding.y * 2 - 1));
	ItemSize(check_bb, style.FramePadding.y);

	//frame
	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0)
	{
		ItemSize(ImVec2(total_bb.GetWidth(), total_bb.GetHeight()), style.FramePadding.y);
		total_bb.Add(total_bb);
	}

	if (!ItemAdd(total_bb, 0))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

	// frame display
	const ImU32 col = GetColorU32(
		(hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button, 
		(hovered && held) ? 1.0f : hovered ? 1.0f : 0.0f);
	window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, col, style.FrameRounding);
	
	if (label_size.x > 0.0f)
	{
		if (hovered)
		{
			PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		RenderText(total_bb.Min + ImVec2(style.FramePadding.x * 2.0f, style.FramePadding.y*0.5f), g.TempBuffer, text_end);

		if (hovered)
		{
			PopStyleColor();
		}
	}

	return pressed;
}

bool ImGui::InputText_Validation(const char* label, char* buf, size_t buf_size, 
	const bool *vValidation, const char* vValidationHelp,
	ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	if (vValidation)
	{
		if (*vValidation)
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
		else
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
	}
	bool res = ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
	if (vValidation)
	{
		ImGui::PopStyleColor();
		if (!*vValidation && ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", vValidationHelp);
	}
	return res;
}

//from imgui_demo.h
void ImGui::HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ImGui::ShowCustomStyleEditor(bool *vOpen, ImGuiStyle* ref)
{
	if (ImGui::Begin("Styles Editor", vOpen))
	{
		// You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
		ImGuiStyle& style = ImGui::GetStyle();
		static ImGuiStyle ref_saved_style;

		// Default to using internal storage as reference
		static bool init = true;
		if (init && ref == nullptr)
			ref_saved_style = style;
		init = false;
		if (ref == nullptr)
			ref = &ref_saved_style;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

		if (ImGui::ShowStyleSelector("Colors##Selector"))
			ref_saved_style = style;
		ImGui::ShowFontSelector("Fonts##Selector");

		// Simplified Settings
		if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
			style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
		{ bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
		ImGui::SameLine();
		{ bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
		ImGui::SameLine();
		{ bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }

		// Save/Revert button
		if (ImGui::Button("Save Ref"))
			*ref = ref_saved_style = style;
		ImGui::SameLine();
		if (ImGui::Button("Revert Ref"))
			style = *ref;
		ImGui::SameLine();
		HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. Use \"Export\" below to save them somewhere.");

		ImGui::Separator();

		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Sizes"))
			{
				ImGui::Text("Main");
				ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
				ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
				ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
				ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
				ImGui::Text("Borders");
				ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
				ImGui::Text("Rounding");
				ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
				ImGui::Text("Alignment");
				ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
				int window_menu_button_position = style.WindowMenuButtonPosition + 1;
				if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
					style.WindowMenuButtonPosition = window_menu_button_position - 1;
				ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
				ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
				ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
				ImGui::Text("Safe Area Padding"); ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
				ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colors"))
			{
				static int output_dest = 0;
				static bool output_only_modified = true;
				if (ImGui::Button("Export"))
				{
					if (output_dest == 0)
						ImGui::LogToClipboard();
					else
						ImGui::LogToTTY();
					ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
					for (int i = 0; i < ImGuiCol_COUNT; i++)
					{
						const ImVec4& col = style.Colors[i];
						const char* name = ImGui::GetStyleColorName(i);
						if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
							ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE, name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
					}
					ImGui::LogFinish();
				}
				ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
				ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

				static ImGuiTextFilter filter;
				filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

				static ImGuiColorEditFlags alpha_flags = 0;
				if (ImGui::RadioButton("Opaque", alpha_flags == 0)) { alpha_flags = 0; } ImGui::SameLine();
				if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
				if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
				HelpMarker("In the color list:\nLeft-click on colored square to open color picker,\nRight-click to open edit options menu.");

				ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
				ImGui::PushItemWidth(-160);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const char* name = ImGui::GetStyleColorName(i);
					if (!filter.PassFilter(name))
						continue;
					ImGui::PushID(i);
					ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
					if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
					{
						// Tips: in a real user application, you may want to merge and use an icon font into the main font, so instead of "Save"/"Revert" you'd use icons.
						// Read the FAQ and docs/FONTS.txt about using icon fonts. It's really easy and super convenient!
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) ref->Colors[i] = style.Colors[i];
						ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) style.Colors[i] = ref->Colors[i];
					}
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
					ImGui::TextUnformatted(name);
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();
}
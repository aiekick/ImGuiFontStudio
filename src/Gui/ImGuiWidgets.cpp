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
#include <ctools/FileHelper.h>
#include <Res/CustomFont.h>

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <Helper/ImGuiThemeHelper.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// https://github.com/ocornut/imgui/issues/3710

inline void PathInvertedRect(ImDrawList *vDrawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
	if (!vDrawList) return;

	rounding = ImMin(rounding, ImFabs(b.x - a.x) * 
		(((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || 
		((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	rounding = ImMin(rounding, ImFabs(b.y - a.y) * 
		(((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || 
		((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	if (rounding <= 0.0f || rounding_corners == 0)
	{
		return;
	}
	else
	{
		const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
		vDrawList->PathLineTo(a);
		vDrawList->PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
		vDrawList->PathFillConvex(col);

		const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
		vDrawList->PathLineTo(ImVec2(b.x, a.y));
		vDrawList->PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
		vDrawList->PathFillConvex(col);

		const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
		vDrawList->PathLineTo(ImVec2(b.x, b.y));
		vDrawList->PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
		vDrawList->PathFillConvex(col);

		const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
		vDrawList->PathLineTo(ImVec2(a.x, b.y));
		vDrawList->PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
		vDrawList->PathFillConvex(col);
	}
}

 void ImGui::AddInvertedRectFilled(ImDrawList* vDrawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners)
{
	if (!vDrawList) return;

	if ((col & IM_COL32_A_MASK) == 0) return;
	if (rounding > 0.0f)
		PathInvertedRect(vDrawList, p_min, p_max, col, rounding, rounding_corners);
}

// Render a rectangle shaped with optional rounding and borders
void ImGui::RenderInnerShadowFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, ImU32 fill_col_darker, ImU32 bg_Color, bool border, float rounding)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
#if 0
	window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding);
#else
	window->DrawList->AddRectFilledMultiColor(p_min, p_max, fill_col, fill_col, fill_col_darker, fill_col_darker);
	AddInvertedRectFilled(window->DrawList, p_min, p_max, bg_Color, rounding, ImDrawCornerFlags_All);
#endif
	const float border_size = g.Style.FrameBorderSize;
	if (border && border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawCornerFlags_All, border_size);
		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, ImDrawCornerFlags_All, border_size);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ImGui::DrawShadowImage(ImTextureID vShadowImage, const ImVec2& vSize, ImU32 col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vSize);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	window->DrawList->AddImage(vShadowImage, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	ImVec2 uvSize = uv1 - uv0;
    float ratioX = ImRatioX(uvSize) * hostRatioX;
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

			ImGui::RenderTextClipped(frame_bb.Min, frame_bb.Max, vLabel, nullptr, &label_size, style.ButtonTextAlign, &frame_bb);

			const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_FrameBg);
			draw_list->AddLine(ImVec2(frame_bb.Min.x + style.ItemInnerSpacing.x, frame_bb.Max.y), ImVec2(frame_bb.Max.x - style.ItemInnerSpacing.x, frame_bb.Max.y), lineCol);
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

	//const ImU32 frameCol = ImGui::GetColorU32(ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? ImGuiCol_TabUnfocusedActive : ImGuiCol_TabUnfocused);
	const ImU32 frameCol = ImGui::GetColorU32(ImGuiCol_TableHeaderBg); 
	ImGui::RenderFrame(p_min, p_max, frameCol, true, style.FrameRounding);

	draw_list->ChannelsMerge(); // merge layers
}

void ImGui::FramedGroupSeparator()
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	
	ImRect bb;
	bb.Min.x = window->WorkRect.Min.x;
	bb.Min.y = window->DC.CursorPos.y;
	bb.Max.x = window->WorkRect.Max.x;
	bb.Max.y = window->DC.CursorPos.y + style.FramePadding.y;

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (ImGui::ItemAdd(bb, 0))
	{
		const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_FrameBg);
		window->DrawList->AddLine(
			ImVec2(bb.Min.x + style.ItemInnerSpacing.x, bb.Max.y - style.FramePadding.y * 0.5f), 
			ImVec2(bb.Max.x - style.ItemInnerSpacing.x, bb.Max.y - style.FramePadding.y * 0.5f), lineCol);
		if (g.LogEnabled)
			LogRenderedText(&bb.Min, "--------------------------------");
	}
}

void ImGui::FramedGroupText(const char* vFmt, ...)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	va_list args;
	va_start(args, vFmt);
	static char TempBuffer[2048] = "\0";
	int w = vsnprintf(TempBuffer, 2046, vFmt, args);
	if (w)
	{
		TempBuffer[w + 1] = '\0'; // 2046 + 1 = 2047 => ok (under array size of 2048 in any case)
		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(TempBuffer);
		const ImGuiStyle& style = g.Style;
		const ImVec2 label_size = ImGui::CalcTextSize(TempBuffer, nullptr, true);

		const float frame_height =
			ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y),
				label_size.y + style.FramePadding.y);
		ImRect bb;
		bb.Min.x = window->WorkRect.Min.x;
		bb.Min.y = window->DC.CursorPos.y;
		bb.Max.x = window->WorkRect.Max.x;
		bb.Max.y = window->DC.CursorPos.y + frame_height;

		ImGui::ItemSize(bb, style.FramePadding.y);
		if (ImGui::ItemAdd(bb, id))
		{
			ImGui::RenderTextClipped(bb.Min, bb.Max, TempBuffer, nullptr, &label_size, style.ButtonTextAlign, &bb);
		}
	}
	va_end(args);
}


void ImGui::FramedGroupText(ImVec4 vTextColor, const char* vFmt, ...)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	va_list args;
	va_start(args, vFmt);
	static char TempBuffer[2048] = "\0";
	int w = vsnprintf(TempBuffer, 2046, vFmt, args);
	if (w)
	{
		TempBuffer[w + 1] = '\0'; // 2046 + 1 = 2047 => ok (under array size of 2048 in any case)
		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(TempBuffer);
		const ImGuiStyle& style = g.Style;
		const ImVec2 label_size = ImGui::CalcTextSize(TempBuffer, nullptr, true);

		const float frame_height =
			ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y),
				label_size.y + style.FramePadding.y);
		ImRect bb;
		bb.Min.x = window->WorkRect.Min.x;
		bb.Min.y = window->DC.CursorPos.y;
		bb.Max.x = window->WorkRect.Max.x;
		bb.Max.y = window->DC.CursorPos.y + frame_height;

		ImGui::ItemSize(bb, style.FramePadding.y);
		if (ImGui::ItemAdd(bb, id))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, vTextColor);
			ImGui::RenderTextClipped(bb.Min, bb.Max, TempBuffer, nullptr, &label_size, style.ButtonTextAlign, &bb);
			ImGui::PopStyleColor();
		}
	}
	va_end(args);
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
	ImVec2 bb_size = ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    //bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
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
	//g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));

	// Get our rectangle in the work area, and report the size we need for next frame.
	// We don't attempt to calculate our height ahead, as it depends on the per-viewport font size. However menu-bar will affect the minimum window size so we'll get the right height.
	static float menuBarH = 0.0f; // will be 0 at first frame
	ImVec2 menu_bar_size = ImVec2(viewport->Size.x - viewport->CurrWorkOffsetMin.x + viewport->CurrWorkOffsetMax.x, 0.0f);
	ImVec2 menu_bar_pos = viewport->Pos + viewport->CurrWorkOffsetMin + ImVec2(0.0f, viewport->Size.y - menuBarH);
	SetNextWindowPos(menu_bar_pos);
	SetNextWindowSize(menu_bar_size);
	SetNextWindowViewport(viewport->ID); // Enforce viewport so we don't create our own viewport when ImGuiConfigFlags_ViewportsNoMerge is set.
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));    // Lift normal size constraint, however the presence of a menu-bar will give us the minimum height we want.
	ImGuiWindowFlags window_flags = 
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	bool is_open = Begin("MainStatusBar", nullptr, window_flags) && BeginMenuBar();
	PopStyleVar(2);

	auto curWin = ImGui::GetCurrentWindowRead();
	if (curWin)
		menuBarH = curWin->MenuBarHeight();

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

IMGUI_API bool ImGui::CustomButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderNavHighlight(bb, id);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed;
}

#ifdef USE_GRADIENT

void ImGui::RenderGradFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_start_col, ImU32 fill_end_col, bool border, float rounding)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	window->DrawList->AddRectFilledMultiColor(p_min, p_max, fill_end_col, fill_end_col, fill_start_col, fill_start_col);
	const float border_size = g.Style.FrameBorderSize;
	if (border && border_size > 0.0f)
	{
		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawCornerFlags_All, border_size);
		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, ImDrawCornerFlags_All, border_size);
	}
}

bool ImGui::GradButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	//const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	//RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	
	const ImVec4 col = GetStyleColorVec4((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImVec4 col_darker = ImVec4(col.x * 0.5f, col.y * 0.5f, col.z * 0.5f, col.w);
	RenderGradFrame(bb.Min, bb.Max, GetColorU32(col_darker), GetColorU32(col), true, g.Style.FrameRounding);

	RenderNavHighlight(bb, id);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SLIDERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a slider, they are all required.
// Read code of e.g. SliderFloat(), SliderInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
// text on the left in the box for keep space
// value on the right in the box
bool ImGui::SliderScalarCompact(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = width - style.FramePadding.x * 2.0f;

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
	
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	// Default format string when passing NULL
	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;
	else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
		IM_ASSERT(0 && "DragInt(): Invalid format string!");

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	const bool hovered = ItemHoverable(total_bb, id);
	bool temp_input_is_active = TempInputIsActive(id);
	if (!temp_input_is_active)
	{
		const bool focus_requested = FocusableItemRegister(window, id);
		const bool clicked = (hovered && g.IO.MouseClicked[0]);
		if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
		{
			SetActiveID(id, window);
			SetFocusID(id, window);
			FocusWindow(window);
			g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			if (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id)
			{
				temp_input_is_active = true;
				FocusableItemUnregister(window);
			}
		}
	}

	// Our current specs do NOT clamp when using CTRL+Click manual input, but we should eventually add a flag for that..
	if (temp_input_is_active)
		return TempInputScalar(total_bb, id, label, data_type, p_data, format);// , p_min, p_max);

	// Draw frame

#ifndef USE_GRADIENT
	const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	RenderNavHighlight(total_bb, id);
	RenderFrame(total_bb.Min, total_bb.Max, frame_col, true, g.Style.FrameRounding);
#else
	const ImVec4 col = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	float sha = ImGuiThemeHelper::Instance()->m_ShadowStrength;
	ImVec4 col_darker = ImVec4(col.x * sha, col.y * sha, col.z * sha, col.w * 0.9f);
	const ImU32 c = GetColorU32(col);
	const ImU32 cd = GetColorU32(col_darker);
	window->DrawList->AddRectFilledMultiColor(total_bb.Min, grab_bb.Max, c, c, cd, cd);
#endif

	// Slider behavior
	ImRect grab_bb;
	const bool value_changed = SliderBehavior(total_bb, id, data_type, p_data, p_min, p_max, format, ImGuiSliderFlags_None, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// Render grab
	if (grab_bb.Max.x > grab_bb.Min.x)
	{
#ifdef USE_GRADIENT
		const ImVec4 col = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
		float sha = ImGuiThemeHelper::Instance()->m_ShadowStrength;
		ImVec4 col_darker = ImVec4(col.x * sha, col.y * sha, col.z * sha, col.w * 0.9f);
		const ImU32 c = GetColorU32(col);
		const ImU32 cd = GetColorU32(col_darker);
		window->DrawList->AddRectFilledMultiColor(grab_bb.Min, grab_bb.Max, c, c, cd, cd);
#else
		window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
#endif
	}

	// display label / but not if input mode 
	if (label_size.x > 0.0f && !temp_input_is_active)
		RenderTextClipped(total_bb.Min + ImVec2(style.ItemInnerSpacing.x, 0), total_bb.Max, label, NULL, NULL, ImVec2(0.0f, 0.5f));

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
	RenderTextClipped(total_bb.Min, total_bb.Max - ImVec2(style.ItemInnerSpacing.x, 0), value_buf, value_buf_end, NULL, ImVec2(1.0f, 0.5f));

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return value_changed;
}

bool ImGui::SliderUIntCompact(float width,
	const char* label, uint32_t* v, uint32_t v_min,
	uint32_t v_max, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_U32, v, &v_min, &v_max, format);
}

bool ImGui::SliderIntCompact(float width,
	const char* label, int* v, int v_min,
	int v_max, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

bool ImGui::SliderFloatCompact(float width,
	const char* label, float* v, float v_min,
	float v_max, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_Float, v, &v_min, &v_max, format);
}

bool ImGui::SliderUIntDefaultCompact(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_default, const char* format)
{
	bool change = false;

	float ax = ImGui::GetCursorPosX();

	ImGui::PushID(label);
	if (CustomButton(ICON_IGFS_RESET))
	{
		*v = v_default;
		change = true;
	}
	ImGui::PopID();

	ImGui::SameLine();

	float w = width - ImGui::GetCursorPosX() + ax;

	change |= SliderScalarCompact(w, label, ImGuiDataType_U32, v, &v_min, &v_max, format);

	return change;
}

bool ImGui::SliderIntDefaultCompact(float width, const char* label, int* v, int v_min, int v_max, int v_default, const char* format)
{
	bool change = false;

	float ax = ImGui::GetCursorPosX();

	ImGui::PushID(label);
	if (CustomButton(ICON_IGFS_RESET))
	{
		*v = v_default;
		change = true;
	}
	ImGui::PopID();

	ImGui::SameLine();

	float w = width - ImGui::GetCursorPosX() + ax;

	change |= SliderScalarCompact(w, label, ImGuiDataType_S32, v, &v_min, &v_max, format);

	return change;
}

bool ImGui::SliderFloatDefaultCompact(float width, const char* label, float* v, float v_min, float v_max, float v_default, const char* format)
{
	bool change = false;

	float ax = ImGui::GetCursorPosX();

	ImGui::PushID(label);
	if (CustomButton(ICON_IGFS_RESET))
	{
		*v = v_default;
		change = true;
	}
	ImGui::PopID();

	ImGui::SameLine();

	float w = width - ImGui::GetCursorPosX() + ax;

	change |= SliderScalarCompact(w, label, ImGuiDataType_Float, v, &v_min, &v_max, format);

	return change;
}

bool ImGui::TransparentButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	//RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	return pressed;
}
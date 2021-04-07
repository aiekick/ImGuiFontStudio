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
#include "ImWidgets.h"

#include <imgui/imgui_widgets.cpp>

#include <ctools/FileHelper.h>
#include <Res/CustomFont.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#define STARTING_CUSTOMID 135
#define RESET_BUTTON_ICON ICON_IGFS_RESET
#define ADD_BUTTON_ICON ICON_IGFS_ADD
#define REMOVE_BUTTON_ICON ICON_IGFS_REMOVE

/////////////////////////////////////
/////////////////////////////////////

float ImGui::CustomStyle::puContrastRatio = 4.5f;
ImU32 ImGui::CustomStyle::puContrastedTextColor = 0;
int ImGui::CustomStyle::pushId = STARTING_CUSTOMID;
int ImGui::CustomStyle::minorNumber = 0;
int ImGui::CustomStyle::majorNumber = 0;
int ImGui::CustomStyle::buildNumber = 0;
ImVec4 ImGui::CustomStyle::GoodColor = ImVec4(0.2f, 0.8f, 0.2f, 0.8f);
ImVec4 ImGui::CustomStyle::BadColor = ImVec4(0.8f, 0.2f, 0.2f, 0.8f);
ImVec4 ImGui::CustomStyle::GlyphButtonColor = ImVec4(0, 0, 0, 1);
ImVec4 ImGui::CustomStyle::GlyphButtonColorActive = ImVec4(0, 0, 0, 1);

void ImGui::CustomStyle::Init()
{
	puContrastedTextColor = ImGui::GetColorU32(ImVec4(0, 0, 0, 1));
	puContrastRatio = 3.0f;
}
void ImGui::CustomStyle::ResetCustomId()
{
	pushId = STARTING_CUSTOMID;
}

void ImGui::SetContrastRatio(float vRatio)
{
	CustomStyle::puContrastRatio = vRatio;
}

void ImGui::SetContrastedTextColor(ImU32 vColor)
{
	CustomStyle::puContrastedTextColor = vColor;
}

void ImGui::DrawContrastWidgets()
{
	ImGui::SliderFloatDefaultCompact(200.0f, "Contrast Ratio", &CustomStyle::puContrastRatio, 0.0f, 21.0f, 3.0f, 0.0f);

	static ImVec4 contrastedTextColor = ImVec4(0, 0, 0, 1);
	if (ImGui::ColorPicker4("Contrasted Text Color", &contrastedTextColor.x))
	{
		CustomStyle::puContrastedTextColor = ImGui::ColorConvertFloat4ToU32(contrastedTextColor);
	}
}

/////////////////////////////////////
/////////////////////////////////////

int ImGui::IncPUSHID()
{
	return ++ImGui::CustomStyle::pushId;
}

int ImGui::GetPUSHID()
{
	return ImGui::CustomStyle::pushId;
}

void ImGui::SetPUSHID(int vID)
{
	ImGui::CustomStyle::pushId = vID;
}

ImVec4 ImGui::GetGoodOrBadColorForUse(bool vUsed)
{
	if (vUsed)
		return ImGui::CustomStyle::GoodColor;
	return ImGui::CustomStyle::BadColor;
}

// viewport mode : 
// if not called from a ImGui Window, will return ImVec2(0,0)
// if its your case, you need to set the GLFWwindow
// no issue withotu viewport
ImVec2 ImGui::GetLocalMousePos(GLFWwindow *vWin)
{
#ifdef IMGUI_HAS_VIEWPORT
	if (vWin)
	{
		double mouse_x, mouse_y;
		glfwGetCursorPos(vWin, &mouse_x, &mouse_y);
		return ImVec2((float)mouse_x, (float)mouse_y);
	}
	else
	{
		ImGuiContext& g = *GImGui;
		auto viewport = g.CurrentViewport;
		if (!viewport && g.CurrentWindow)
			viewport = g.CurrentWindow->Viewport;
		if (viewport && viewport->PlatformHandle)
		{
			double mouse_x, mouse_y;
			glfwGetCursorPos((GLFWwindow*)viewport->PlatformHandle, &mouse_x, &mouse_y);
			return ImVec2((float)mouse_x, (float)mouse_y);
		}
	}
#else
	return ImGui::GetMousePos();
#endif
	return ImVec2(0.0f, 0.0f);
}

/////////////////////////////////////
/////////////////////////////////////

// contrast from 1 to 21
// https://www.w3.org/TR/WCAG20/#relativeluminancedef
float ImGui::CalcContrastRatio(const ImU32& backgroundColor, const ImU32& foreGroundColor)
{
	const float sa0 = (float)((backgroundColor >> IM_COL32_A_SHIFT) & 0xFF);
	const float sa1 = (float)((foreGroundColor >> IM_COL32_A_SHIFT) & 0xFF);
	static float sr = 0.2126f / 255.0f;
	static float sg = 0.7152f / 255.0f;
	static float sb = 0.0722f / 255.0f;
	const float contrastRatio =
		(sr * sa0 * ((backgroundColor >> IM_COL32_R_SHIFT) & 0xFF) +
			sg * sa0 * ((backgroundColor >> IM_COL32_G_SHIFT) & 0xFF) +
			sb * sa0 * ((backgroundColor >> IM_COL32_B_SHIFT) & 0xFF) + 0.05f) /
		(sr * sa1 * ((foreGroundColor >> IM_COL32_R_SHIFT) & 0xFF) +
			sg * sa1 * ((foreGroundColor >> IM_COL32_G_SHIFT) & 0xFF) +
			sb * sa1 * ((foreGroundColor >> IM_COL32_B_SHIFT) & 0xFF) + 0.05f);
	if (contrastRatio < 1.0f)
		return 1.0f / contrastRatio;
	return contrastRatio;
}

bool ImGui::PushStyleColorWithContrast(const ImGuiCol& backGroundColor, const ImGuiCol& foreGroundColor, const ImU32& invertedColor, const float& maxContrastRatio)
{
	const float contrastRatio = CalcContrastRatio(ImGui::GetColorU32(backGroundColor), ImGui::GetColorU32(foreGroundColor));
	if (contrastRatio < maxContrastRatio)
	{
		ImGui::PushStyleColor(foreGroundColor, invertedColor);
		return true;
	}
	return false;
}

bool ImGui::PushStyleColorWithContrast(const ImGuiCol& backGroundColor, const ImGuiCol& foreGroundColor, const ImVec4& invertedColor, const float& maxContrastRatio)
{
	const float contrastRatio = CalcContrastRatio(ImGui::GetColorU32(backGroundColor), ImGui::GetColorU32(foreGroundColor));
	if (contrastRatio < maxContrastRatio)
	{
		ImGui::PushStyleColor(foreGroundColor, invertedColor);
		return true;
	}
	return false;
}

bool ImGui::PushStyleColorWithContrast(const ImU32& backGroundColor, const ImGuiCol& foreGroundColor, const ImU32& invertedColor, const float& maxContrastRatio)
{
	const float contrastRatio = CalcContrastRatio(backGroundColor, ImGui::GetColorU32(foreGroundColor));
	if (contrastRatio < maxContrastRatio)
	{
		ImGui::PushStyleColor(foreGroundColor, invertedColor);
		return true;
	}
	return false;
}

bool ImGui::PushStyleColorWithContrast(const ImU32& backGroundColor, const ImGuiCol& foreGroundColor, const ImVec4& invertedColor, const float& maxContrastRatio)
{
	const float contrastRatio = CalcContrastRatio(backGroundColor, ImGui::GetColorU32(foreGroundColor));
	if (contrastRatio < maxContrastRatio)
	{
		ImGui::PushStyleColor(foreGroundColor, invertedColor);
		return true;
	}
	return false;
}

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

	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vSize);
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

    const ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, nullptr, true);
    const ImVec2 btn_label_size = btnLabel ? CalcTextSize(btnLabel, nullptr, true) : ImVec2(0, 0);
    const ImVec2 size(size_arg.x > 0.0f ? size_arg.x : label_size.x, size_arg.y > 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    const ImRect bb_inner(pos, pos + size);
    ItemSize(size, 0.0f);

    // Fill horizontal space.
    const ImVec2 window_padding = window->WindowPadding;
    const float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    const float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
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

    bool itepuadd;
    if (flags & ImGuiSelectableFlags_Disabled)
    {
	    const ImGuiItemFlags backup_itepuflags = window->DC.ItemFlags;
        window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
        itepuadd = ItemAdd(bb, id);
        window->DC.ItemFlags = backup_itepuflags;
    }
    else
    {
        itepuadd = ItemAdd(bb, id);
    }
    if (!itepuadd)
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
    const bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

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
        const ImGuiID extraId = window->GetID((void*)(intptr_t)(id + 1));
        const ImVec2 p = ImVec2(bb.Max.x, bb.Min.y + btn_label_size.y * 0.33f);
        const ImRect bbMenu(p, p + btn_label_size);

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
    const bool pressed = ButtonBehavior(bb, id, &hovered, &held);

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
    const ImVec2 uvSize = uv1 - uv0;
    const float ratioX = ImRatioX(uvSize) * hostRatioX;
    const ImVec2 imgSize = image_bb.GetSize();
    const float newX = imgSize.y * ratioX;
    ImVec2 glyphSize = ImVec2(imgSize.x, imgSize.x / ratioX) * 0.5f;
    if (newX < imgSize.x) glyphSize = ImVec2(newX, imgSize.y) * 0.5f;
    const ImVec2 center = image_bb.GetCenter();
    window->DrawList->AddImage(user_texture_id, center - glyphSize, center + glyphSize, uv0, uv1, GetColorU32(ImGuiCol_Text));

    return pressed;
}

bool ImGui::BeginFramedGroup(const char *vLabel, ImGuiCol vHoveredIdx, ImGuiCol NormalIdx)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->ChannelsSplit(2); // split for have 2 layers

    draw_list->ChannelsSetCurrent(1); // Layer ForeGround

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImGui::BeginGroup();
	auto res = FramedGroupButtonText(vLabel);
	
	window->ContentRegionRect.Max.x -= style.FramePadding.x * 3.0f;
	window->WorkRect.Max.x -= style.FramePadding.x * 3.0f;

	const ImGuiID id = window->GetID(vLabel);
	window->DC.StateStorage->SetInt(id + 1, vHoveredIdx);
	window->DC.StateStorage->SetInt(id + 2, NormalIdx);
	window->DC.StateStorage->SetBool(id + 3, res);
	window->IDStack.push_back(id);

	ImGui::Indent();

	if (!res)
	{
		EndFramedGroup();
	}

    return res;
}

void ImGui::EndFramedGroup()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto id = window->IDStack.back();
	window->IDStack.pop_back();
	
	bool expanded = window->DC.StateStorage->GetBool(id + 3);

	ImGui::Unindent();
	if (expanded)
		ImGui::Spacing();
	ImGui::EndGroup();
	
	window->ContentRegionRect.Max.x += style.FramePadding.x * 3.0f;
	window->WorkRect.Max.x += style.FramePadding.x * 3.0f;

	draw_list->ChannelsSetCurrent(0); // Layer Background

    ImVec2 p_min = ImGui::GetItemRectMin();
    p_min.x = window->WorkRect.Min.x;
    
	ImVec2 p_max = ImGui::GetItemRectMax();
    p_max.x = window->WorkRect.Max.x;

	ImGuiCol hoveredIdx = window->DC.StateStorage->GetInt(id + 1);
	ImGuiCol normalIdx = window->DC.StateStorage->GetInt(id + 2);
	
	const ImU32 frameCol = ImGui::GetColorU32(ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? hoveredIdx : normalIdx);
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
	bb.Min.x = window->DC.CursorPos.x;
	bb.Min.y = window->DC.CursorPos.y;
	bb.Max.x = window->WorkRect.Max.x;
	bb.Max.y = window->DC.CursorPos.y + style.FramePadding.y;

	ImGui::ItemSize(bb, style.FramePadding.y);
	if (ImGui::ItemAdd(bb, 0))
	{
		const ImU32 lineCol = ImGui::GetColorU32(ImGuiCol_FrameBg);
		window->DrawList->AddLine(
			ImVec2(bb.Min.x, bb.Max.y - style.FramePadding.y * 0.5f), 
			ImVec2(bb.Max.x, bb.Max.y - style.FramePadding.y * 0.5f), lineCol);
		if (g.LogEnabled)
			LogRenderedText(&bb.Min, "--------------------------------");
	}
}

void ImGui::FramedGroupText(ImVec4* vTextColor, const char* vHelp, const char* vFmt, va_list vArgs)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	static char TempBuffer[2048] = "\0";
	int w = vsnprintf(TempBuffer, 2046, vFmt, vArgs);
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

		ImGui::ItemSize(bb, 0.0f);
		if (ImGui::ItemAdd(bb, id))
		{
			if (vTextColor)
				ImGui::PushStyleColor(ImGuiCol_Text, *vTextColor);
			ImGui::RenderTextClipped(bb.Min, bb.Max, TempBuffer, nullptr, &label_size, style.ButtonTextAlign, &bb);
			if (vTextColor)
				ImGui::PopStyleColor();
		}
	}

	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", vHelp);
}

bool ImGui::FramedGroupButtonText(const char* vFmt, ...)
{
	bool res = false;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return res;

	static char TempBuffer[2048] = "\0";
	va_list args;
	va_start(args, vFmt); 
	int w = vsnprintf(TempBuffer, 2046, vFmt, args);
	va_end(args);
	if (w)
	{
		TempBuffer[w + 1] = '\0'; // 2046 + 1 = 2047 => ok (under array size of 2048 in any case)
		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(TempBuffer);
		const ImGuiStyle& style = g.Style;
		const ImVec2 label_size = ImGui::CalcTextSize(TempBuffer, nullptr, true);
		const float frame_height =
			ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y),
				label_size.y + style.FramePadding.y * 2.0f);

		ImRect bb;
		bb.Min.x = window->WorkRect.Min.x;
		bb.Min.y = window->DC.CursorPos.y;
		bb.Max.x = window->WorkRect.Max.x;
		bb.Max.y = window->DC.CursorPos.y + frame_height;

		const float text_base_offset_y = ImMax(0.0f, window->DC.CurrLineTextBaseOffset - style.FramePadding.y); // Latch before ItemSize changes it
		res = TreeNodeBehaviorIsOpen(id, ImGuiTreeNodeFlags_DefaultOpen); // a faire avant le ItemAdd

		ImGui::ItemSize(bb, 0.0f);
		if (!ImGui::ItemAdd(bb, id))
			return res;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
		if (pressed)
		{
			res = !res;
			window->DC.StateStorage->SetInt(id, res);
		}

		// Render
		const ImU32 colArrow = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		ImGui::RenderFrame(bb.Min, bb.Max, colArrow, true, style.FrameRounding);
		ImGui::RenderTextClipped(bb.Min, bb.Max, TempBuffer, nullptr, &label_size, style.ButtonTextAlign, &bb);
		RenderArrow(window->DrawList, bb.Min + style.FramePadding + ImVec2(0.0f, text_base_offset_y), ImGui::GetColorU32(ImGuiCol_Text),
			(res ? ImGuiDir_::ImGuiDir_Down : ImGuiDir_::ImGuiDir_Right), 1.0f);

		ImGui::RenderTextClipped(bb.Min, bb.Max, TempBuffer, nullptr, &label_size, style.ButtonTextAlign, &bb);
	}

	return res;
}

void ImGui::FramedGroupText(const char* vFmt, ...)
{
	va_list args;
	va_start(args, vFmt);
	FramedGroupText(nullptr, 0, vFmt, args);
	va_end(args);
}

// renamed because "FramedGroupTextHelp(const char* vHelp, const char* vFmt" 
// can be choosed by the compiler for "FramedGroupText(const char* vFmt"
void ImGui::FramedGroupTextHelp(const char* vHelp, const char* vFmt, ...)
{
	va_list args;
	va_start(args, vFmt);
	FramedGroupText(nullptr, vHelp, vFmt, args);
	va_end(args);
}

void ImGui::FramedGroupText(ImVec4 vTextColor, const char* vFmt, ...)
{
	va_list args;
	va_start(args, vFmt);
	FramedGroupText(&vTextColor, 0, vFmt, args);
	va_end(args);
}

bool ImGui::CheckBoxBoolDefault(const char* vName, bool* vVar, bool vDefault, const char* vHelp, ImFont* vLabelFont)
{
	bool change = false;

	//float padding = ImGui::GetStyle().FramePadding.x;

	change = ImGui::ContrastedButton(RESET_BUTTON_ICON, "Reset", vLabelFont);
	if (change)
		*vVar = vDefault;

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(++CustomStyle::pushId);

	change |= ImGui::Checkbox(vName, vVar);

	ImGui::PopID();

	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(vHelp);

	return change;
}

bool ImGui::CheckBoxFloat(const char* vName, float* vVar)
{
	bool change = false;

	ImGui::PushID(ImGui::IncPUSHID());
	bool v = (*vVar > 0.5);
	if (ImGui::Checkbox(vName, &v))
	{
		change = true;
		*vVar = v ? 1.0f : 0.0f;
	}
	ImGui::PopID();

	return change;
}

bool ImGui::CheckBoxFloatDefault(const char* vName, float* vVar, float vDefault, const char* vHelp, ImFont* vLabelFont)
{
	bool change = false;

	change = ImGui::ContrastedButton(RESET_BUTTON_ICON, "Reset", vLabelFont);
	if (change)
		*vVar = vDefault;

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	change |= CheckBoxFloat(vName, vVar);

	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(vHelp);

	return change;
}

bool ImGui::RadioFloatDefault(const char* vName, float* vVar, int vCount, float* vDefault, const char* vHelp, ImFont* vLabelFont)
{
	bool change = false;

	//float padding = ImGui::GetStyle().FramePadding.x;

	ImGui::BeginGroup();
	{
		change = ImGui::ContrastedButton(RESET_BUTTON_ICON, "Reset", vLabelFont);
		if (change)
			*vVar = *vDefault;

		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

		int radioSelectedId = 0;
		for (int i = 0; i < vCount; i++)
		{
			if (i > 0) ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::PushID(ImGui::IncPUSHID());
			bool v = (vVar[i] > 0.5f);
			if (ImGui::Checkbox("##radio", &v))
			{
				radioSelectedId = i;
				change = true;
			}
			ImGui::PopID();
		}

		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

		ImGui::Text(vName);

		if (change)
		{
			for (int j = 0; j < vCount; j++)
			{
				vVar[j] = (radioSelectedId == j ? 1.0f : 0.0f);
			}
		}
	}
	ImGui::EndGroup();

	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(vHelp);

	return change;
}

bool ImGui::RadioButtonLabeled(float vWidth, const char* label, bool active, bool disabled)
{
    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    float w = vWidth;
	const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	if (w < 0.0f) w = ImGui::GetContentRegionMaxAbs().x - window->DC.CursorPos.x;
	if (IS_FLOAT_EQUAL(w, 0.0f)) w = label_size.x + style.FramePadding.x * 2.0f;
	const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    // check
	bool pressed = false;
	ImGuiCol colUnderText = ImGuiCol_FrameBg;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		colUnderText = ImGuiCol_FrameBg;
		window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : colUnderText), style.FrameRounding);
		if (active)
		{
			colUnderText = ImGuiCol_Button;
			window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : colUnderText), style.FrameRounding);
		}
	}
#ifdef _DEBUG
	else
	{
		//CTOOL_DEBUG_BREAK;
	}
#endif
    // circle shadow + bg
    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddRect(total_bb.Min + ImVec2(1, 1), total_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(total_bb.Min, total_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (label_size.x > 0.0f)
    {
		const bool pushed = ImGui::PushStyleColorWithContrast(colUnderText, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);
		RenderTextClipped(total_bb.Min, total_bb.Max, label, nullptr, &label_size, ImVec2(0.5f, 0.5f));
		if (pushed)
			ImGui::PopStyleColor();
		
    }

    return pressed;
}

bool ImGui::RadioButtonLabeled(float vWidth, const char* label, const char* help, bool active, bool disabled, ImFont* vLabelFont)
{
	if (vLabelFont)	ImGui::PushFont(vLabelFont);
	const bool change = RadioButtonLabeled(vWidth, label, active, disabled);
	if (vLabelFont)	ImGui::PopFont();
	if (help)
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", help);
    return change;
}

bool ImGui::RadioButtonLabeled(float vWidth, const char* label, const char* help, bool *active, bool disabled, ImFont* vLabelFont)
{
	if (vLabelFont)	ImGui::PushFont(vLabelFont);
	const bool change = RadioButtonLabeled(vWidth, label, *active, disabled);
	if (vLabelFont)	ImGui::PopFont();
	if (change)
        *active = !*active;
    if (help)
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", help);
    return change;
}

bool ImGui::CollapsingHeader_SmallHeight(const char *vName, float vHeightRatio, float vWidth, bool *vIsOpen)
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

    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 nsize = ImGui::CalcItemSize(
            ImVec2(vWidth, label_size.y + padding.y * 2.0f),
            label_size.x + padding.x * 2.0f, label_size.y + padding.y * 2.0f);
    //nsize.y *= vHeightRatio;
	bool is_open = TreeNodeBehaviorIsOpen(id, ImGuiNextItemDataFlags_HasOpen); // a faire avant le ItemAdd

    ImRect bb(pos, pos + nsize);
    const ImRect bbTrigger = bb;
    ImGui::ItemSize(bb, padding.y);
    ImGui::ItemAdd(bb, id);

     if (vIsOpen && !*vIsOpen) is_open = false;
    is_open = window->DC.StateStorage->GetInt(id, is_open ? 1 : 0) != 0;
    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bbTrigger, id, &hovered, &held, 0);
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

bool ImGui::CollapsingHeader_CheckBox(const char* vName, float vWidth, bool vShowCheckBox, bool* vCheckCatched)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(vName);
	ImVec2 label_size = ImGui::CalcTextSize(vName, nullptr, true);
	//label_size.x = ImMin(label_size.x, vWidth);
	const ImVec2 padding = ImGui::GetStyle().FramePadding;
	const float text_base_offset_y = ImMax(0.0f, window->DC.CurrLineTextBaseOffset - padding.y); // Latch before ItemSize changes it

	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 nsize = ImGui::CalcItemSize(ImVec2(vWidth, label_size.y + style.FramePadding.y * 2.0f), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
	bool is_open = TreeNodeBehaviorIsOpen(id, ImGuiNextItemDataFlags_HasOpen); // a faire avant le ItemAdd

	ImRect bb(pos, pos + nsize);
	ImRect bbTrigger = bb;
	if (vShowCheckBox && vCheckCatched != nullptr)
		bbTrigger.Max.x -= nsize.y;
	ImGui::ItemSize(bb, style.FramePadding.y);
	ImGui::ItemAdd(bb, id);

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bbTrigger, id, &hovered, &held, 0);
	//ImGui::SetItemAllowOverlap();
	if (pressed)
	{
		is_open = !is_open;
		window->DC.StateStorage->SetInt(id, is_open);
	}

	// Render
	const ImU32 colArrow = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
	ImGui::RenderFrame(bb.Min, bb.Max, colArrow, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, vName, nullptr, &label_size, style.ButtonTextAlign, &bb);
	RenderArrow(window->DrawList, bb.Min + padding + ImVec2(0.0f, text_base_offset_y), ImGui::GetColorU32(ImGuiCol_Text),
		(is_open ? ImGuiDir_::ImGuiDir_Down : ImGuiDir_::ImGuiDir_Right), 1.0f);

	// menu
	if (vShowCheckBox && vCheckCatched)
	{
		// item
		const ImGuiID extraId = window->GetID((void*)(intptr_t)(id + 1));
		ImRect bbMenu(ImVec2(bb.Max.x - nsize.y, bb.Min.y), bb.Max);
		bbMenu.Min.y += nsize.y * 0.1f;
		bbMenu.Max.x -= nsize.y * 0.1f;
		bbMenu.Max.y -= nsize.y * 0.1f;

		// detection
		pressed = ImGui::ButtonBehavior(bbMenu, extraId, &hovered, &held, 0);
		if (pressed)
		{
			*vCheckCatched = !(*vCheckCatched);
			ImGui::MarkItemEdited(extraId);
		}

		// render
		const ImU32 colFrame = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		ImGui::RenderFrame(bbMenu.Min, bbMenu.Max, colFrame, true, style.FrameRounding);
		//ImVec2 iconPos = ImVec2(pos.x + nsize.x - nsize.y * 0.5f, pos.y + nsize.y * 0.5f);
		if (*vCheckCatched)
		{
			const float sizey = nsize.y;
			const float pad = ImMax(1.0f, (float)(int)(sizey / 6.0f));
			ImGui::RenderCheckMark(window->DrawList, bbMenu.Min + ImVec2(pad, pad - 0.1f * sizey), ImGui::GetColorU32(ImGuiCol_CheckMark), sizey - pad * 2.0f);
		}
	}

	return is_open;
}

bool ImGui::CollapsingHeader_Button(const char* vName, float vWidth, const char* vLabelButton, bool vShowButton, bool* vButtonPressed, ImFont* vButtonFont)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(vName);
	ImVec2 label_size = ImGui::CalcTextSize(vName, nullptr, true);
	const ImVec2 padding = ImGui::GetStyle().FramePadding;
	const float text_base_offset_y = ImMax(0.0f, window->DC.CurrLineTextBaseOffset - padding.y); // Latch before ItemSize changes it

	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 nsize = ImGui::CalcItemSize(ImVec2(vWidth, label_size.y + style.FramePadding.y * 2.0f), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
	bool is_open = TreeNodeBehaviorIsOpen(id, ImGuiNextItemDataFlags_HasOpen); // a faire avant le ItemAdd

	ImRect bb(pos, pos + nsize);
	ImRect bbTrigger = bb;
	if (vButtonPressed && vShowButton)
		bbTrigger.Max.x -= nsize.y;
	ImGui::ItemSize(bb, style.FramePadding.y);
	ImGui::ItemAdd(bb, id);

	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(bbTrigger, id, &hovered, &held, 0);
	if (pressed)
	{
		is_open = !is_open;
		window->DC.StateStorage->SetInt(id, is_open);
	}

	// Render
	const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, vName, nullptr, &label_size, style.ButtonTextAlign, &bb);
	RenderArrow(window->DrawList, bb.Min + padding + ImVec2(0.0f, text_base_offset_y), ImGui::GetColorU32(ImGuiCol_Text),
		(is_open ? ImGuiDir_::ImGuiDir_Down : ImGuiDir_::ImGuiDir_Right), 1.0f);

	// menu
	if (vButtonPressed && vShowButton)
	{
		// item
		const ImGuiID extraId = window->GetID((void*)(intptr_t)(id + 1));
		const ImRect bbMenu(ImVec2(bb.Max.x - nsize.y, bb.Min.y), bb.Max);

		// detection
		bool menuHovered, menuHeld;
		*vButtonPressed |= ImGui::ButtonBehavior(bbMenu, extraId, &menuHovered, &menuHeld, 0);
		const ImU32 menuCol = ImGui::GetColorU32((menuHovered) ? ImGuiCol_ButtonHovered : menuHovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);

		// render
		ImGui::RenderFrame(bbMenu.Min, bbMenu.Max, menuCol, true, style.FrameRounding);
		const ImVec2 iconPos = ImVec2(pos.x + nsize.x - nsize.y * 0.5f, pos.y + nsize.y * 0.5f);

		const float cross_extent = (nsize.y * 0.5f * 0.7071f) - 1.0f;
		window->DrawList->AddLine(iconPos + ImVec2(+cross_extent * 0.2f, -cross_extent * 0.2f), iconPos + ImVec2(-cross_extent, +cross_extent), ImGui::GetColorU32(ImGuiCol_Text), 4.0f);
		window->DrawList->AddLine(iconPos + ImVec2(+cross_extent * 0.4f, -cross_extent * 0.4f), iconPos + ImVec2(+cross_extent, -cross_extent), ImGui::GetColorU32(ImGuiCol_Text), 4.0f);
	}

	return is_open;
}

bool ImGui::ButtonNoFrame(const char* vLabel, ImVec2 size, ImVec4 vColor, const char* vHelp, ImFont* vLabelFont)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	//ImGuiContext& g = *GImGui;
	ImGui::PushID(++CustomStyle::pushId);
	const ImGuiID id = window->GetID(vLabel);
	ImGui::PopID();
	const float h = ImGui::GetFrameHeight();
	const ImVec2 sz = ImMax(ImVec2(h, h), size);
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + sz);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	if (vLabelFont) ImGui::PushFont(vLabelFont);
	ImGui::PushStyleColor(ImGuiCol_Text, vColor);
	RenderTextClipped(bb.Min, bb.Max, vLabel, nullptr, nullptr, ImVec2(0.5f, 0.5f));
	ImGui::PopStyleColor();
	if (vLabelFont)	ImGui::PopFont();
	
	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(vHelp);

	return pressed;
}

bool ImGui::SmallContrastedButton(const char* label)
{
	ImGuiContext& g = *GImGui;
	float backup_padding_y = g.Style.FramePadding.y;
	g.Style.FramePadding.y = 0.0f;
	const bool pushed = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);
	ImGui::PushID(++CustomStyle::pushId);
	bool pressed = ButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
	ImGui::PopID();
	if (pushed)
		ImGui::PopStyleColor();
	g.Style.FramePadding.y = backup_padding_y;
	return pressed;
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
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, 0.0f);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	const ImGuiButtonFlags flags = 0;
	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
	if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	ImGui::RenderNavHighlight(bb, id);
	ImVec4 defColor = ImGui::GetStyleColorVec4(ImGuiCol_Text); defColor.w = 0.5f;
	ImVec4 hovColor = defColor; hovColor.w = 0.75f;
	ImVec4 actColor = defColor; actColor.w = 1.0f;
	const ImVec4 col = (hovered && held) ? actColor : hovered ? hovColor : defColor;
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

bool ImGui::ClickableTextFile(const char* label, const char* file, bool vOnlined)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 size = ImGui::CalcItemSize(ImVec2(0.0f, 0.0f), label_size.x + style.FramePadding.x * 1.0f, label_size.y);
	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, 0.0f);
	if (!ImGui::ItemAdd(bb, id))
		return false;
	const ImGuiButtonFlags flags = 0;
	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);
	if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id))
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
	ImGui::RenderNavHighlight(bb, id);
	ImVec4 defColor = ImGui::GetStyleColorVec4(ImGuiCol_Text); defColor.w = 0.5f;
	ImVec4 hovColor = defColor; hovColor.w = 0.75f;
	ImVec4 actColor = defColor; actColor.w = 1.0f;
	const ImVec4 col = (hovered && held) ? actColor : hovered ? hovColor : defColor;
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
		ImGui::SetTooltip("%s", file);
	}

	if (pressed)
	{
		FileHelper::Instance()->OpenFile(file);
	}

    return pressed;
}

bool ImGui::ColorEdit3Default(float vWidth, const char* vName, float* vCol, float* vDefault)
{
	bool change = false;

	float padding = ImGui::GetStyle().FramePadding.x;

	if (!ImGui::GetCurrentWindow()->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	ImGui::PushID(ImGui::IncPUSHID());
	change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
	ImGui::PopID();
	float w = vWidth - ImGui::GetItemRectSize().x - padding * 2.0f;
	if (change)
	{
		vCol[0] = vDefault[0];
		vCol[1] = vDefault[1];
		vCol[2] = vDefault[2];
	}

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(ImGui::IncPUSHID());
	ImGui::PushItemWidth(w);
	change |= ImGui::ColorEdit3(vName, vCol, ImGuiColorEditFlags_Float);
	ImGui::PopItemWidth();
	ImGui::PopID();

	return change;
}
bool ImGui::ColorEdit4Default(float vWidth, const char* vName, float* vCol, float* vDefault)
{
	bool change = false;

	float padding = ImGui::GetStyle().FramePadding.x;

	if (!ImGui::GetCurrentWindow()->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	ImGui::PushID(ImGui::IncPUSHID());
	change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
	ImGui::PopID();
	float w = vWidth - ImGui::GetItemRectSize().x - padding * 2.0f;
	if (change)
	{
		vCol[0] = vDefault[0];
		vCol[1] = vDefault[1];
		vCol[2] = vDefault[2];
		vCol[3] = vDefault[3];
	}

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(ImGui::IncPUSHID());
	ImGui::PushItemWidth(w);
	change |= ImGui::ColorEdit4(vName, vCol, ImGuiColorEditFlags_Float);
	ImGui::PopItemWidth();
	ImGui::PopID();

	return change;
}

void ImGui::Header(const char* vName, float width)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(vName);
	const ImVec2 label_size = ImGui::CalcTextSize(vName, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(ImVec2(width, label_size.y + style.FramePadding.y * 2.0f), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return;

	bool hovered, held;
	/*bool pressed = */ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);
	ImGui::SetItemAllowOverlap();

	// Render
	const ImU32 col = ImGui::GetColorU32(hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, vName, NULL, &label_size, style.ButtonTextAlign, &bb);
}

void ImGui::ImageRect(ImTextureID user_texture_id, const ImVec2& pos, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + pos + size);
	if (border_col.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	if (border_col.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
		window->DrawList->AddImage(user_texture_id, bb.Min + pos + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
	}
	else
	{
		window->DrawList->AddImage(user_texture_id, bb.Min + pos, bb.Max, uv0, uv1, GetColorU32(tint_col));
	}
}


bool ImGui::ColorEdit3ForNode(const char* label, float col[3], ImGuiColorEditFlags flags)
{
	return ColorEdit4ForNode(label, col, flags | ImGuiColorEditFlags_NoAlpha);
}

// Edit colors components (each component in 0.0f..1.0f range).
// See enum ImGuiColorEditFlags_ for available options. e.g. Only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// With typical options: Left-click on colored square to open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool ImGui::ColorEdit4ForNode(const char* label, float col[4], ImGuiColorEditFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float square_sz = GetFrameHeight();
	const float w_full = CalcItemWidth();
	const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
	const float w_inputs = w_full - w_button;
	const char* label_display_end = FindRenderedTextEnd(label);
	g.NextItemData.ClearFlags();

	BeginGroup();
	PushID(label);

	// If we're not showing any slider there's no point in doing any HSV conversions
	const ImGuiColorEditFlags flags_untouched = flags;
	if (flags & ImGuiColorEditFlags_NoInputs)
		flags = (flags & (~ImGuiColorEditFlags__DisplayMask)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

	// Context menu: display and modify options (before defaults are applied)
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		ColorEditOptionsPopup(col, flags);

	// Read stored options
	if (!(flags & ImGuiColorEditFlags__DisplayMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DisplayMask);
	if (!(flags & ImGuiColorEditFlags__DataTypeMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
	if (!(flags & ImGuiColorEditFlags__PickerMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
	if (!(flags & ImGuiColorEditFlags__InputMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputMask);
	flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__DisplayMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags__InputMask));
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__DisplayMask)); // Check that only 1 is selected
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));   // Check that only 1 is selected

	const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
	const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
	const int components = alpha ? 4 : 3;

	// Convert to the formats we need
	float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
	if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
		ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
	else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
	{
		// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
		ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
		if (memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0)
		{
			if (f[1] == 0)
				f[0] = g.ColorEditLastHue;
			if (f[2] == 0)
				f[1] = g.ColorEditLastSat;
		}
	}
	int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

	bool value_changed = false;
	bool value_changed_as_float = false;

	const ImVec2 pos = window->DC.CursorPos;
	const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
	window->DC.CursorPos.x = pos.x + inputs_offset_x;

	if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB/HSV 0..255 Sliders
		const float w_item_one = ImMax(1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
		const float w_item_last = ImMax(1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

		const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
		static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
		static const char* fmt_table_int[3][4] =
		{
			{   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
			{ "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
			{ "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
		};
		static const char* fmt_table_float[3][4] =
		{
			{   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
			{ "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
			{ "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
		};
		const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;

		for (int n = 0; n < components; n++)
		{
			//if (n > 0)
			//	SameLine(0, style.ItemInnerSpacing.x);

			SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

			// FIXME: When ImGuiColorEditFlags_HDR flag is passed HS values snap in weird ways when SV values go below 0.
			if (flags & ImGuiColorEditFlags_Float)
			{
				value_changed |= DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
				value_changed_as_float |= value_changed;
			}
			else
			{
				value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
			}
			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context");
		}
	}
	else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
	{
		// RGB Hexadecimal Input
		char buf[64];
		if (alpha)
			ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
		else
			ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));
		SetNextItemWidth(w_inputs);
		if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
		{
			value_changed = true;
			char* p = buf;
			while (*p == '#' || ImCharIsBlankA(*p))
				p++;
			i[0] = i[1] = i[2] = i[3] = 0;
			if (alpha)
				sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
			else
				sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");
	}

	ImGuiWindow* picker_active_window = NULL;
	if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
	{
		const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
		window->DC.CursorPos = ImVec2(pos.x + button_offset_x, pos.y);

		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
		if (ColorButton("##ColorButton", col_v4, flags))
		{
			if (!(flags & ImGuiColorEditFlags_NoPicker))
			{
				// Store current color and open a picker
				g.ColorPickerRef = col_v4;
				OpenPopup("picker");
				SetNextWindowPos(window->DC.LastItemRect.GetBL() + ImVec2(-1, style.ItemSpacing.y));
			}
		}
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			OpenPopupOnItemClick("context");

		if (BeginPopup("picker"))
		{
			picker_active_window = g.CurrentWindow;
			if (label != label_display_end)
			{
				TextEx(label, label_display_end);
				Spacing();
			}
			ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
			ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__DisplayMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
			SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
			value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
			EndPopup();
		}
	}

	if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
	{
		const float text_offset_x = (flags & ImGuiColorEditFlags_NoInputs) ? w_button : w_full + style.ItemInnerSpacing.x;
		window->DC.CursorPos = ImVec2(pos.x + text_offset_x, pos.y + style.FramePadding.y);
		TextEx(label, label_display_end);
	}

	// Convert back
	if (value_changed && picker_active_window == NULL)
	{
		if (!value_changed_as_float)
			for (int n = 0; n < 4; n++)
				f[n] = i[n] / 255.0f;
		if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
		{
			g.ColorEditLastHue = f[0];
			g.ColorEditLastSat = f[1];
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
			memcpy(g.ColorEditLastColor, f, sizeof(float) * 3);
		}
		if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
			ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

		col[0] = f[0];
		col[1] = f[1];
		col[2] = f[2];
		if (alpha)
			col[3] = f[3];
	}

	PopID();
	EndGroup();

	// Drag and Drop Target
	// NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
	if ((window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
	{
		bool accepted_drag_drop = false;
		if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512
			value_changed = accepted_drag_drop = true;
		}
		if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
		{
			memcpy((float*)col, payload->Data, sizeof(float) * components);
			value_changed = accepted_drag_drop = true;
		}

		// Drag-drop payloads are always RGB
		if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
			ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
		EndDragDropTarget();
	}

	// When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
	if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
		window->DC.LastItemId = g.ActiveId;

	if (value_changed)
		MarkItemEdited(window->DC.LastItemId);

	return value_changed;
}

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
	const char* statusBarLabel = "##MainStatusBar";

	ImGuiContext& g = *GImGui;
	ImGuiViewportP* viewport = g.Viewports[0];
	ImGuiWindow* status_bar_window = FindWindowByName(statusBarLabel);

	ImVec2 status_bar_pos, status_bar_size;

	// Get our rectangle at the top of the work area
	if (status_bar_window == nullptr)
	{
		status_bar_pos = viewport->Pos + viewport->CurrWorkOffsetMin;
		status_bar_pos.y += viewport->Size.y - viewport->CurrWorkOffsetMax.y - 1.0f;
		status_bar_size = ImVec2(viewport->Size.x - viewport->CurrWorkOffsetMin.x + viewport->CurrWorkOffsetMax.x, 1.0f);
	}
	else
	{
		status_bar_pos = viewport->Pos + viewport->CurrWorkOffsetMax;
		status_bar_pos.y += viewport->Size.y - viewport->CurrWorkOffsetMax.y - status_bar_window->Size.y;
		status_bar_size = ImVec2(viewport->Size.x - viewport->CurrWorkOffsetMin.x + viewport->CurrWorkOffsetMax.x, status_bar_window->Size.y);
	}

	// Get our rectangle in the work area, and report the size we need for next frame.
	// We don't attempt to calculate our height ahead, as it depends on the per-viewport font size. However menu-bar will affect the minimum window size so we'll get the right height.
	SetNextWindowPos(status_bar_pos);
	SetNextWindowSize(status_bar_size);
	SetNextWindowViewport(viewport->ID); // Enforce viewport so we don't create our own viewport when ImGuiConfigFlags_ViewportsNoMerge is set.
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));    // Lift normal size constraint, however the presence of a menu-bar will give us the minimum height we want.
	const ImGuiWindowFlags window_flags = 
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	const bool is_open = Begin(statusBarLabel, nullptr, window_flags) && BeginMenuBar();
	PopStyleVar(2);

	// Report our size into work area (for next frame) using actual window size
	status_bar_window = GetCurrentWindow();
	if (status_bar_window->BeginCount == 1)
		viewport->CurrWorkOffsetMax.y -= status_bar_window->Size.y;

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

bool ImGui::BeginLeftToolBar(float vWidth)
{
	const char* barLabel = "##LeftToolBar";

	ImGuiContext& g = *GImGui;
	ImGuiViewportP* viewport = g.Viewports[0];
	ImGuiWindow* menu_bar_window = FindWindowByName(barLabel);

	// Get our rectangle at the top of the work area
	if (menu_bar_window == nullptr || menu_bar_window->BeginCount == 0)
	{
		// Set window position
		// We don't attempt to calculate our height ahead, as it depends on the per-viewport font size. However menu-bar will affect the minimum window size so we'll get the right height.
		const ImVec2 menu_bar_pos = viewport->Pos + viewport->CurrWorkOffsetMin;
		const ImVec2 menu_bar_size = ImVec2(vWidth, viewport->Size.y - viewport->CurrWorkOffsetMin.y + viewport->CurrWorkOffsetMax.y);
		SetNextWindowPos(menu_bar_pos);
		SetNextWindowSize(menu_bar_size);
	}

	// Create window
	PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));    // Lift normal size constraint, however the presence of a menu-bar will give us the minimum height we want.
	const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
	const bool is_open = Begin(barLabel, nullptr, window_flags);
	PopStyleVar(2);

	// Report our size into work area (for next frame) using actual window size
	menu_bar_window = GetCurrentWindow();
	if (menu_bar_window->BeginCount == 1)
		viewport->CurrWorkOffsetMin.x += menu_bar_window->Size.x;

	return is_open; //-V1020
}

void ImGui::EndLeftToolBar()
{
	End();
}

bool ImGui::ContrastedButton(const char* label, const char* help, ImFont* imfont, float vWidth, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	const bool pushed = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);

	if (imfont)
		ImGui::PushFont(imfont);

	ImGui::PushID(++CustomStyle::pushId);

	const bool res = ImGui::ButtonEx(label, ImVec2(ImMax(vWidth, size_arg.x), size_arg.y), flags);

	ImGui::PopID();

	if (imfont)
		ImGui::PopFont();

	if (pushed)
		ImGui::PopStyleColor();

	if (help)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", help);

	return res;
}

bool ImGui::ToggleContrastedButton(const char* vLabelTrue, const char* vLabelFalse, bool* vValue, const char* vHelp, ImFont* vImfont)
{
	bool res = false;

	assert(vValue);

	const auto pushed = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);

	if (vImfont)
		ImGui::PushFont(vImfont);

	ImGui::PushID(++CustomStyle::pushId);

	if (ImGui::Button(*vValue ? vLabelTrue : vLabelFalse))
	{
		*vValue = !*vValue;
		res = true;
	}

	ImGui::PopID();

	if (vImfont)
		ImGui::PopFont();

	if (pushed)
		ImGui::PopStyleColor();

	if (vHelp)
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", vHelp);

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

	const ImVec2 label_size = CalcTextSize(g.TempBuffer, text_end, true);

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
	const bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

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

void ImGui::PlotFVec4Histo(const char* vLabel, ct::fvec4* vDatas, int vDataCount, bool* vShowChannel, ImVec2 frame_size, ct::fvec4 scale_min, ct::fvec4 scale_max, int* vHoveredIdx)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(vLabel);

	const ImVec2 label_size = ImGui::CalcTextSize(vLabel, NULL, true);
	if (IS_FLOAT_EQUAL(frame_size.x, 0.0f))
		frame_size.x = ImGui::CalcItemWidth();
	if (IS_FLOAT_EQUAL(frame_size.y, 0.0f))
		frame_size.y = label_size.y + (style.FramePadding.y * 2);

	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
	ImGui::ItemSize(total_bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
		return;

	ImGui::PushID(++CustomStyle::pushId);

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	// Determine scale from values if not specified
	for (int chan = 0; chan < 4; chan++)
	{
		if (IS_FLOAT_EQUAL(scale_min[chan], FLT_MAX) || IS_FLOAT_EQUAL(scale_max[chan], FLT_MAX))
		{
			float v_min = FLT_MAX;
			float v_max = -FLT_MAX;
			for (int i = 0; i < vDataCount; i++)
			{
				const float v = vDatas[i][chan];
				v_min = ct::mini(v_min, v);
				v_max = ct::maxi(v_max, v);
			}
			if (scale_min[chan] == FLT_MAX)
				scale_min.Set(chan, v_min);
			if (scale_max[chan] == FLT_MAX)
				scale_max.Set(chan, v_max);
		}
	}

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	const int values_count_min = 2;
	if (vDataCount >= values_count_min)
	{
		int res_w = ct::mini((int)frame_size.x, (int)vDataCount) - 1;
		int item_count = vDataCount - 1;

		// Tooltip on hover
		int v_hovered = -1;
		if (vHoveredIdx)
			*vHoveredIdx = v_hovered;

		if (hovered && inner_bb.Contains(g.IO.MousePos))
		{
			const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
			const int v_idx = (int)(t * item_count);
			IM_ASSERT(v_idx >= 0 && v_idx < vDataCount);

			const ct::fvec4 v0 = vDatas[v_idx % vDataCount];
			ImGui::BeginTooltip();
			if (!vShowChannel || vShowChannel[0])
			{
				ImGui::Text("r %d: %8.4g", v_idx, v0.x);
			}
			if (!vShowChannel || vShowChannel[1])
			{
				ImGui::Text("g %d: %8.4g", v_idx, v0.y);
			}
			if (!vShowChannel || vShowChannel[2])
			{
				ImGui::Text("b %d: %8.4g", v_idx, v0.z);
			}
			if (!vShowChannel || vShowChannel[3])
			{
				ImGui::Text("a %d: %8.4g", v_idx, v0.w);
			}
			ImGui::EndTooltip();
			v_hovered = v_idx;

			if (vHoveredIdx)
				*vHoveredIdx = v_hovered;
		}

		float t_step = 1.0f / (float)res_w;
		ct::fvec4 inv_scale;
		inv_scale.x = (scale_min.x == scale_max.x) ? 0.0f : (1.0f / (scale_max.x - scale_min.x));
		inv_scale.y = (scale_min.y == scale_max.y) ? 0.0f : (1.0f / (scale_max.y - scale_min.y));
		inv_scale.z = (scale_min.z == scale_max.z) ? 0.0f : (1.0f / (scale_max.z - scale_min.z));
		inv_scale.w = (scale_min.w == scale_max.w) ? 0.0f : (1.0f / (scale_max.w - scale_min.w));

		const ct::fvec4 v0 = (vDatas[0] - scale_min) * inv_scale;
		ct::fvec4 yt0;
		yt0.x = 1.0f - ct::clamp(v0.x);
		yt0.y = 1.0f - ct::clamp(v0.y);
		yt0.z = 1.0f - ct::clamp(v0.z);
		yt0.w = 1.0f - ct::clamp(v0.w);
		float xt0 = 0.0f;

		ct::fvec4 histogram_zero_line_t;
		histogram_zero_line_t.x = (scale_min.x * scale_max.x < 0.0f) ? (-scale_min.x * inv_scale.x) : (scale_min.x < 0.0f ? 0.0f : 1.0f); // Where does the zero line stands
		histogram_zero_line_t.y = (scale_min.y * scale_max.y < 0.0f) ? (-scale_min.y * inv_scale.y) : (scale_min.y < 0.0f ? 0.0f : 1.0f); // Where does the zero line stands
		histogram_zero_line_t.z = (scale_min.z * scale_max.z < 0.0f) ? (-scale_min.z * inv_scale.z) : (scale_min.z < 0.0f ? 0.0f : 1.0f); // Where does the zero line stands
		histogram_zero_line_t.w = (scale_min.w * scale_max.w < 0.0f) ? (-scale_min.w * inv_scale.w) : (scale_min.w < 0.0f ? 0.0f : 1.0f); // Where does the zero line stands

		for (int n = 0; n < res_w; n++)
		{
			float xt1 = xt0 + t_step;
			int v1_idx = (int)(xt0 * item_count + 0.5f);
			IM_ASSERT(v1_idx >= 0 && v1_idx < vDataCount);
			const ct::fvec4 v1 = (vDatas[(v1_idx + 1) % vDataCount] - scale_min) * inv_scale;
			ct::fvec4 yt1;// = 1.0f - ct::clamp(v1);
			yt1.x = 1.0f - ct::clamp(v1.x);
			yt1.y = 1.0f - ct::clamp(v1.y);
			yt1.z = 1.0f - ct::clamp(v1.z);
			yt1.w = 1.0f - ct::clamp(v1.w);

			if (!vShowChannel || vShowChannel[0])
			{
				ImVec2 p0x = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.x));
				ImVec2 p1x = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.x));
				window->DrawList->AddLine(p0x, p1x, ImGui::GetColorU32(ImVec4(1, 0, 0, 1)));
			}

			if (!vShowChannel || vShowChannel[1])
			{
				ImVec2 p0y = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.y));
				ImVec2 p1y = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.y));
				window->DrawList->AddLine(p0y, p1y, ImGui::GetColorU32(ImVec4(0, 1, 0, 1)));
			}

			if (!vShowChannel || vShowChannel[2])
			{
				ImVec2 p0z = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.z));
				ImVec2 p1z = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.z));
				window->DrawList->AddLine(p0z, p1z, ImGui::GetColorU32(ImVec4(0, 0, 1, 1)));
			}

			if (!vShowChannel || vShowChannel[3])
			{
				ImVec2 p0w = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.w));
				ImVec2 p1w = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.w));
				window->DrawList->AddLine(p0w, p1w, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
			}

			xt0 = xt1;
			yt0 = yt1;
		}

		if (!vShowChannel || vShowChannel[0])
		{
			float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.x);
			char buf[20]; snprintf(buf, 20, "r %.3f", yt0.x);
			window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), buf);
		}

		if (!vShowChannel || vShowChannel[1])
		{
			float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.y);
			char buf[20]; snprintf(buf, 20, "g %.3f", yt0.y);
			window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(0, 1, 0, 1)), buf);
		}

		if (!vShowChannel || vShowChannel[2])
		{
			float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.z);
			char buf[20]; snprintf(buf, 20, "b %.3f", yt0.z);
			window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), buf);
		}

		if (!vShowChannel || vShowChannel[3])
		{
			float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.w);
			char buf[20]; snprintf(buf, 20, "a %.3f", yt0.w);
			window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)), buf);
		}
	}

	ImGui::PopID();
}

void ImGui::ImageZoomPoint(ImTextureID vUserTextureId, const float vWidth, const ImVec2& vCenter, const ImVec2& vPoint, const ImVec2& vRadiusInPixels)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImVec2 size = ImVec2(vWidth, vWidth);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	ImVec2 rp = ImVec2(vRadiusInPixels.x + 1.0f, vRadiusInPixels.y + 1.0f);

	ImVec2 uv0 = vCenter - vRadiusInPixels * vPoint;
	ImVec2 uv1 = vCenter + rp * vPoint;

	window->DrawList->AddImage(vUserTextureId, bb.Min, bb.Max, uv0, uv1);

	ImVec2 s = size * vRadiusInPixels / (rp + vRadiusInPixels);

	ImVec2 a = bb.Min + s;
	ImVec2 b = bb.Max - s;

	window->DrawList->AddRect(a, b, ImGui::GetColorU32(ImVec4(1, 1, 0, 1)));
}

/*
void ImGui::ImageZoomLine(ImTextureID vUserTextureId, const float vWidth, const ImVec2& vStart, const ImVec2& vEnd)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImVec2 size = ImVec2(vWidth, vWidth);
	float w = std::fabs(vEnd.x - vStart.x);
	float h = std::fabs((1.0f - vEnd.y) - (1.0f - vStart.y));
	float ratioX = (float)h / (float)w;
	float y = size.x * ratioX;
	if (y > size.y)
		size.x = size.y / ratioX;
	else
		size.y = y;
	size.x = ct::clamp(size.x, 1.0f, vWidth);
	size.y = ct::clamp(size.y, 1.0f, vWidth);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	window->DrawList->AddImage(vUserTextureId, bb.Min, bb.Max, vStart, vEnd);

	//ImVec2 a = bb.Min + s;
	//ImVec2 b = bb.Max - s;
	//window->DrawList->AddLine(a, b, ImGui::GetColorU32(ImVec4(1, 1, 0, 1)));
}
*/

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
	const bool res = ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
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

bool ImGui::TransparentButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, nullptr, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	const ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	const bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	//RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, nullptr, &label_size, style.ButtonTextAlign, &bb);

	return pressed;
}

void ImGui::PlainImageWithBG(ImTextureID vTexId, const ImVec2& size, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_col), 0.0f);
	window->DrawList->AddImage(vTexId, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), GetColorU32(tint_col));
}

void ImGui::ImageRatio(ImTextureID vTexId, float vRatioX, float vWidth, ImVec4 vColor, float /*vBorderThick*/)
{
	if (vTexId == 0)
		return;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	if (!window->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vWidth);

	const float ratioX = vRatioX;
	const float y = size.x * ratioX;
	if (y > size.y)
		size.x = size.y / ratioX;
	else
		size.y = y;

	size.x = ct::clamp(size.x, 1.0f, vWidth);
	size.y = ct::clamp(size.y, 1.0f, vWidth);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (vColor.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	if (vColor.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vColor), 0.0f);
		window->DrawList->AddImage(vTexId, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}
	else
	{
		window->DrawList->AddImage(vTexId, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

#ifdef _DEBUG
	if (ImGui::IsItemHovered())
	{
		char arr[3];
		if (snprintf(arr, 3, "%i", (int)(size_t)vTexId))
		{
			ImGui::SetTooltip(arr);
		}
	}
#endif
}

#ifdef USE_OPENGL
bool ImGui::TextureOverLay(float vWidth, ct::texture* vTex, ImVec4 vBorderColor, const char* vOverlayText, ImVec4
	vOverLayTextColor, ImVec4 vOverLayBgColor)
{
	if (vTex == nullptr)
		return false;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	if (!window->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vWidth);

	const float ratioX = (float)vTex->h / (float)vTex->w;
	const float y = size.x * ratioX;
	if (y > size.y)
		size.x = size.y / ratioX;
	else
		size.y = y;

	size.x = ct::clamp(size.x, 1.0f, vWidth);
	size.y = ct::clamp(size.y, 1.0f, vWidth);

	const ImGuiID id = window->GetID(vTex);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (vBorderColor.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	//if (pressed)
		//CTOOL_DEBUG_BREAK;

	if (vBorderColor.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vBorderColor), 0.0f, 15, vBorderColor.w);
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}
	else
	{
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

	ImVec2 center = (bb.Max + bb.Min) * 0.5f;
	ImGuiContext& g = *GImGui;
	const float fontSize = 40.0f;
	const ImVec2 ls = ImGui::CalcTextSize(vOverlayText, nullptr, 0) * fontSize / g.Font->FontSize;
	center = ImVec2(center.x - ls.x * 0.5f, center.y - ls.y * 0.5f);

	if (hovered)
	{
		window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(vOverLayBgColor));
		window->DrawList->AddText(nullptr, fontSize, center, ImGui::GetColorU32(vOverLayTextColor), vOverlayText);
		//ImGui::RenderTextClipped(bb.Min, bb.Max, vOverlayText, 0, 0, ImVec2(0.5f, 0.5f), &bb);
	}
	else
	{
		// bug fix de type flickering https://github.com/ocornut/imgui/issues/2506
		//window->DrawList->AddText(0, fontSize, center, ImGui::GetColorU32(ImVec4(0, 0, 0, 1)), ".");
	}


#ifdef _DEBUG
	/*if (ImGui::IsItemHovered())
	{
		char arr[3];
		if (snprintf(arr, 3, "%i", (int)vTex->glTex))
		{
			ImGui::SetTooltip(arr);
		}
	}*/
#endif

	return pressed;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SLIDERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// FIXME: Move more of the code into SliderBehavior()
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
inline bool inSliderBehaviorStepperT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const TYPE v_step, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
	using namespace ImGui;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
	const bool is_decimal = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
	const bool is_logarithmic = (flags & ImGuiSliderFlags_Logarithmic) && is_decimal;

	const float grab_padding = 2.0f;
	const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
	float grab_sz = style.GrabMinSize;
	SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
	if (!is_decimal && v_range >= 0)                                             // v_range < 0 may happen on integer overflows
		grab_sz = ImMax((float)(slider_sz / (v_range + 1)), style.GrabMinSize);  // For integer sliders: if possible have the grab size represent 1 unit
	grab_sz = ImMin(grab_sz, slider_sz);
	const float slider_usable_sz = slider_sz - grab_sz;
	const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
	const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

	float logarithmic_zero_epsilon = 0.0f; // Only valid when is_logarithmic is true
	float zero_deadzone_halfsize = 0.0f; // Only valid when is_logarithmic is true
	if (is_logarithmic)
	{
		// When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of clamp value greatly affects slider precision. We attempt to use the specified precision to estimate a good lower bound.
		const int decimal_precision = is_decimal ? ImParseFormatPrecision(format, 3) : 1;
		logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);
		zero_deadzone_halfsize = (style.LogSliderDeadzone * 0.5f) / ImMax(slider_usable_sz, 1.0f);
	}

	// Process interacting with the slider
	bool value_changed = false;
	if (g.ActiveId == id)
	{
		bool set_new_value = false;
		float clicked_t = 0.0f;
		if (g.ActiveIdSource == ImGuiInputSource_Mouse)
		{
			if (!g.IO.MouseDown[0])
			{
				ClearActiveID();
			}
			else
			{
				const float mouse_abs_pos = g.IO.MousePos[axis];
				clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
				if (axis == ImGuiAxis_Y)
					clicked_t = 1.0f - clicked_t;
				set_new_value = true;
			}
		}
		else if (g.ActiveIdSource == ImGuiInputSource_Nav)
		{
			if (g.ActiveIdIsJustActivated)
			{
				g.SliderCurrentAccum = 0.0f; // Reset any stored nav delta upon activation
				g.SliderCurrentAccumDirty = false;
			}

			const ImVec2 input_delta2 = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard | ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_RepeatFast, 0.0f, 0.0f);
			float input_delta = (axis == ImGuiAxis_X) ? input_delta2.x : -input_delta2.y;
			if (input_delta != 0.0f)
			{
				const int decimal_precision = is_decimal ? ImParseFormatPrecision(format, 3) : 0;
				if (decimal_precision > 0)
				{
					input_delta /= 100.0f;    // Gamepad/keyboard tweak speeds in % of slider bounds
					if (IsNavInputDown(ImGuiNavInput_TweakSlow))
						input_delta /= 10.0f;
				}
				else
				{
					if ((v_range >= -100.0f && v_range <= 100.0f) || IsNavInputDown(ImGuiNavInput_TweakSlow))
						input_delta = ((input_delta < 0.0f) ? -1.0f : +1.0f) / (float)v_range; // Gamepad/keyboard tweak speeds in integer steps
					else
						input_delta /= 100.0f;
				}
				if (IsNavInputDown(ImGuiNavInput_TweakFast))
					input_delta *= 10.0f;

				g.SliderCurrentAccum += input_delta;
				g.SliderCurrentAccumDirty = true;
			}

			float delta = g.SliderCurrentAccum;
			if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
			{
				ClearActiveID();
			}
			else if (g.SliderCurrentAccumDirty)
			{
				clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

				if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
				{
					set_new_value = false;
					g.SliderCurrentAccum = 0.0f; // If pushing up against the limits, don't continue to accumulate
				}
				else
				{
					set_new_value = true;
					float old_clicked_t = clicked_t;
					clicked_t = ImSaturate(clicked_t + delta);

					// Calculate what our "new" clicked_t will be, and thus how far we actually moved the slider, and subtract this from the accumulator
					TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
					if (!(flags & ImGuiSliderFlags_NoRoundToFormat))
						v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);
					float new_clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_new, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

					if (delta > 0)
						g.SliderCurrentAccum -= ImMin(new_clicked_t - old_clicked_t, delta);
					else
						g.SliderCurrentAccum -= ImMax(new_clicked_t - old_clicked_t, delta);
				}

				g.SliderCurrentAccumDirty = false;
			}
		}

		if (set_new_value)
		{
			TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

			if (v_step)
			{
				FLOATTYPE v_new_off_f = (FLOATTYPE)v_new;
				TYPE v_new_off_floor = (TYPE)((int)(v_new_off_f / v_step) * v_step);
				//TYPE v_new_off_round = v_new_off_floor + v_step;
				v_new = v_new_off_floor;
			}

			// Round to user desired precision based on format string
			if (!(flags & ImGuiSliderFlags_NoRoundToFormat))
				v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);

			// Apply result
			if (*v != v_new)
			{
				*v = v_new;
				value_changed = true;
			}
		}
	}

	if (slider_sz < 1.0f)
	{
		*out_grab_bb = ImRect(bb.Min, bb.Min);
	}
	else
	{
		// Output grab position so it can be displayed by the caller
		float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
		if (axis == ImGuiAxis_Y)
			grab_t = 1.0f - grab_t;
		const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
		if (axis == ImGuiAxis_X)
			*out_grab_bb = ImRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
		else
			*out_grab_bb = ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
	}

	return value_changed;
}

// For 32-bit and larger types, slider bounds are limited to half the natural type range.
// So e.g. an integer Slider between INT_MAX-10 and INT_MAX will fail, but an integer Slider between INT_MAX/2-10 and INT_MAX/2 will be ok.
// It would be possible to lift that limitation with some work but it doesn't seem to be worth it for sliders.
inline bool inSliderBehaviorStepper(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const void* p_step, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
	// Those MIN/MAX values are not define because we need to point to them
	static const signed char    IM_S8_MIN = -128;
	static const signed char    IM_S8_MAX = 127;
	static const unsigned char  IM_U8_MIN = 0;
	static const unsigned char  IM_U8_MAX = 0xFF;
	static const signed short   IM_S16_MIN = -32768;
	static const signed short   IM_S16_MAX = 32767;
	static const unsigned short IM_U16_MIN = 0;
	static const unsigned short IM_U16_MAX = 0xFFFF;
	static const ImS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
	static const ImS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
	static const ImU32          IM_U32_MIN = 0;
	static const ImU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
	static const ImS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
	static const ImS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
	static const ImS64          IM_S64_MIN = -9223372036854775807LL - 1;
	static const ImS64          IM_S64_MAX = 9223372036854775807LL;
#endif
	static const ImU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
	static const ImU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
	static const ImU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

	// Read imgui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
	IM_ASSERT((flags == 1 || (flags & ImGuiSliderFlags_InvalidMask_) == 0) && "Invalid ImGuiSliderFlags flag!  Has the 'float power' argument been mistakenly cast to flags? Call function with ImGuiSliderFlags_Logarithmic flags instead.");

	ImGuiContext& g = *GImGui;
	if ((g.CurrentWindow->DC.ItemFlags & ImGuiItemFlags_ReadOnly) || (flags & ImGuiSliderFlags_ReadOnly))
		return false;

	switch (data_type)
	{
	case ImGuiDataType_S8: { ImS32 v32 = (ImS32) * (ImS8*)p_v;  bool r = inSliderBehaviorStepperT<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, *(const ImS8*)p_min, *(const ImS8*)p_max, *(const ImS8*)p_step, format, flags, out_grab_bb); if (r) *(ImS8*)p_v = (ImS8)v32;  return r; }
	case ImGuiDataType_U8: { ImU32 v32 = (ImU32) * (ImU8*)p_v;  bool r = inSliderBehaviorStepperT<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, *(const ImU8*)p_min, *(const ImU8*)p_max, *(const ImU8*)p_step, format, flags, out_grab_bb); if (r) *(ImU8*)p_v = (ImU8)v32;  return r; }
	case ImGuiDataType_S16: { ImS32 v32 = (ImS32) * (ImS16*)p_v; bool r = inSliderBehaviorStepperT<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, *(const ImS16*)p_min, *(const ImS16*)p_max, *(const ImS16*)p_step, format, flags, out_grab_bb); if (r) *(ImS16*)p_v = (ImS16)v32; return r; }
	case ImGuiDataType_U16: { ImU32 v32 = (ImU32) * (ImU16*)p_v; bool r = inSliderBehaviorStepperT<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, *(const ImU16*)p_min, *(const ImU16*)p_max, *(const ImU16*)p_step, format, flags, out_grab_bb); if (r) *(ImU16*)p_v = (ImU16)v32; return r; }
	case ImGuiDataType_S32:
		IM_ASSERT(*(const ImS32*)p_min >= IM_S32_MIN / 2 && *(const ImS32*)p_max <= IM_S32_MAX / 2);
		return inSliderBehaviorStepperT<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)p_v, *(const ImS32*)p_min, *(const ImS32*)p_max, *(const ImS32*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_U32:
		IM_ASSERT(*(const ImU32*)p_max <= IM_U32_MAX / 2);
		return inSliderBehaviorStepperT<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)p_v, *(const ImU32*)p_min, *(const ImU32*)p_max, *(const ImU32*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_S64:
		IM_ASSERT(*(const ImS64*)p_min >= IM_S64_MIN / 2 && *(const ImS64*)p_max <= IM_S64_MAX / 2);
		return inSliderBehaviorStepperT<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)p_v, *(const ImS64*)p_min, *(const ImS64*)p_max, *(const ImS64*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_U64:
		IM_ASSERT(*(const ImU64*)p_max <= IM_U64_MAX / 2);
		return inSliderBehaviorStepperT<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)p_v, *(const ImU64*)p_min, *(const ImU64*)p_max, *(const ImU64*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_Float:
		IM_ASSERT(*(const float*)p_min >= -FLT_MAX / 2.0f && *(const float*)p_max <= FLT_MAX / 2.0f);
		return inSliderBehaviorStepperT<float, float, float >(bb, id, data_type, (float*)p_v, *(const float*)p_min, *(const float*)p_max, *(const float*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_Double:
		IM_ASSERT(*(const double*)p_min >= -DBL_MAX / 2.0f && *(const double*)p_max <= DBL_MAX / 2.0f);
		return inSliderBehaviorStepperT<double, double, double>(bb, id, data_type, (double*)p_v, *(const double*)p_min, *(const double*)p_max, *(const double*)p_step, format, flags, out_grab_bb);
	case ImGuiDataType_COUNT: break;
	}
	IM_ASSERT(0);
	return false;
}

// Note: p_data, p_min and p_max are _pointers_ to a memory address holding the data. For a slider, they are all required.
// Read code of e.g. SliderFloat(), SliderInt() etc. or examples in 'Demo->Widgets->Data Types' to understand how to use this function directly.
// text on the left in the box for keep space
// value on the right in the box
bool ImGui::SliderScalarCompact(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_step, const char* format)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	float w = width;
	if (width <= 0.0f)
	{
		w = ImGui::GetContentRegionMaxAbs().x - window->DC.CursorPos.x - style.FramePadding.x;
	}

	const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	// Default format string when passing NULL
	if (format == nullptr)
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
#ifdef USE_SHADOW
	if (!ThemeHelper::puUseShadow)
	{
#endif
		const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		RenderNavHighlight(total_bb, id);
		RenderFrame(total_bb.Min, total_bb.Max, frame_col, true, g.Style.FrameRounding);
#ifdef USE_SHADOW
	}
	else
	{
		const ImVec4 col = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		float sha = ThemeHelper::Instance()->puShadowStrength;
		ImVec4 col_darker = ImVec4(col.x * sha, col.y * sha, col.z * sha, col.w * 0.9f);
		const ImU32 c = GetColorU32(col);
		const ImU32 cd = GetColorU32(col_darker);
		window->DrawList->AddRectFilledMultiColor(total_bb.Min, total_bb.Max, c, c, cd, cd);
	}
#endif
	// Slider behavior
	ImRect grab_bb;
	const bool value_changed = inSliderBehaviorStepper(total_bb, id, data_type, p_data, p_min, p_max, p_step, format, ImGuiSliderFlags_None, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// Render grab
	if (grab_bb.Max.x > grab_bb.Min.x)
	{
#ifdef USE_SHADOW
		if (ThemeHelper::puUseShadow)
		{
			const ImVec4 col = GetStyleColorVec4(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
			float sha = ThemeHelper::Instance()->puShadowStrength;
			ImVec4 col_darker = ImVec4(col.x * sha, col.y * sha, col.z * sha, col.w * 0.9f);
			const ImU32 c = GetColorU32(col);
			const ImU32 cd = GetColorU32(col_darker);
			window->DrawList->AddRectFilledMultiColor(grab_bb.Min, grab_bb.Max, c, c, cd, cd);
		}
		else
		{
#endif
			window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
#ifdef USE_SHADOW
		}
#endif
	}

	// display label / but not if input mode 
	if (label_size.x > 0.0f && !temp_input_is_active)
		RenderTextClipped(total_bb.Min + ImVec2(style.ItemInnerSpacing.x, 0), total_bb.Max, label, nullptr, nullptr, ImVec2(0.0f, 0.5f));

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
	RenderTextClipped(total_bb.Min, total_bb.Max - ImVec2(style.ItemInnerSpacing.x, 0), value_buf, value_buf_end, nullptr, ImVec2(1.0f, 0.5f));

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return value_changed;
}

bool ImGui::SliderIntCompact(float width, const char* label, int* v, int v_min, int v_max, int v_step, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_S32, v, &v_min, &v_max, &v_step, format);
}

bool ImGui::SliderUIntCompact(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_step, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_step, format);
}

bool ImGui::SliderSizeTCompact(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_step, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_step, format);
}

bool ImGui::SliderFloatCompact(float width, const char* label, float* v, float v_min, float v_max, float v_step, const char* format)
{
	return SliderScalarCompact(width, label, ImGuiDataType_Float, v, &v_min, &v_max, &v_step, format);
}

bool ImGui::SliderScalarDefaultCompact(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_default, const void* p_step, const char* format)
{
	bool change = false;

	ImGui::PushID(label);
	if (ContrastedButton(RESET_BUTTON_ICON))
	{
		switch (data_type)
		{
		case ImGuiDataType_S8: *(ImS8*)(p_data) = *(ImS8*)(p_default); break;
		case ImGuiDataType_U8: *(ImU8*)(p_data) = *(ImU8*)(p_default); break;
		case ImGuiDataType_S16: *(ImS16*)(p_data) = *(ImS16*)(p_default); break;
		case ImGuiDataType_U16: *(ImU16*)(p_data) = *(ImU16*)(p_default); break;
		case ImGuiDataType_S32: *(ImS32*)(p_data) = *(ImS32*)(p_default); break;
		case ImGuiDataType_U32: *(ImU32*)(p_data) = *(ImU32*)(p_default); break;
		case ImGuiDataType_S64: *(ImS64*)(p_data) = *(ImS64*)(p_default); break;
		case ImGuiDataType_U64: *(ImU64*)(p_data) = *(ImU64*)(p_default); break;
		case ImGuiDataType_Float: *(float*)(p_data) = *(float*)(p_default); break;
		case ImGuiDataType_Double: *(double*)(p_data) = *(double*)(p_default); break;
		case ImGuiDataType_COUNT: break;
		}
		change = true;
	}
	ImGui::PopID();

	ImGui::SameLine();

	if (width > 0.0f)
	{
		width -= ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x;
	}

	change |= SliderScalarCompact(width, label, data_type, p_data, p_min, p_max, p_step, format);

	return change;
}

bool ImGui::SliderIntDefaultCompact(float width, const char* label, int* v, int v_min, int v_max, int v_default, int v_step, const char* format)
{
	return SliderScalarDefaultCompact(width, label, ImGuiDataType_S32, v, &v_min, &v_max, &v_default, &v_step, format);
}

bool ImGui::SliderUIntDefaultCompact(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_default, uint32_t v_step, const char* format)
{
	return SliderScalarDefaultCompact(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_default, &v_step, format);
}

bool ImGui::SliderSizeTDefaultCompact(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_default, size_t v_step, const char* format)
{
	return SliderScalarDefaultCompact(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_default, &v_step, format);
}

bool ImGui::SliderFloatDefaultCompact(float width, const char* label, float* v, float v_min, float v_max, float v_default, float v_step, const char* format)
{
	return SliderScalarDefaultCompact(width, label, ImGuiDataType_Float, v, &v_min, &v_max, &v_default, &v_step, format);
}

bool ImGui::SliderScalar(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_step, const char* format, ImGuiSliderFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	float w = width;
	if (width <= 0.0f)
	{
		w = ImGui::GetContentRegionMaxAbs().x - window->DC.CursorPos.x - style.FramePadding.x;
	}

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id, &frame_bb))
		return false;

	// Default format string when passing NULL
	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;
	else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
		IM_ASSERT(0 && "DragInt(): Invalid format string!");

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	const bool hovered = ItemHoverable(frame_bb, id);
	const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
	bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id);
	if (!temp_input_is_active)
	{
		const bool focus_requested = temp_input_allowed && FocusableItemRegister(window, id);
		const bool clicked = (hovered && g.IO.MouseClicked[0]);
		if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
		{
			SetActiveID(id, window);
			SetFocusID(id, window);
			FocusWindow(window);
			g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
			if (temp_input_allowed && (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
			{
				temp_input_is_active = true;
				FocusableItemUnregister(window);
			}
		}
	}

	if (temp_input_is_active)
	{
		// Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
		const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
		return TempInputScalar(frame_bb, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
	}

	// Draw frame
	const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	RenderNavHighlight(frame_bb, id);
	RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

	// Slider behavior
	ImRect grab_bb;
	const bool value_changed = inSliderBehaviorStepper(frame_bb, id, data_type, p_data, p_min, p_max, p_step, format, flags, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// Render grab
	if (grab_bb.Max.x > grab_bb.Min.x)
		window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
	if (g.LogEnabled)
		LogSetNextTextDecoration("{", "}");
	RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return value_changed;
}

bool ImGui::SliderInt(float width, const char* label, int* v, int v_min, int v_max, int v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalar(width, label, ImGuiDataType_S32, v, &v_min, &v_max, &v_step, format, flags);
}

bool ImGui::SliderUInt(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalar(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_step, format, flags);
}

bool ImGui::SliderSizeT(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalar(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_step, format, flags);
}

bool ImGui::SliderFloat(float width, const char* label, float* v, float v_min, float v_max, float v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalar(width, label, ImGuiDataType_Float, v, &v_min, &v_max, &v_step, format, flags);
}

bool ImGui::SliderScalarDefault(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_default, const void* p_step, const char* format, ImGuiSliderFlags flags)
{
	bool change = false;

	ImGui::PushID(label);
	if (ContrastedButton(RESET_BUTTON_ICON))
	{
		switch (data_type)
		{
		case ImGuiDataType_S8: *(ImS8*)(p_data) = *(ImS8*)(p_default); break;
		case ImGuiDataType_U8: *(ImU8*)(p_data) = *(ImU8*)(p_default); break;
		case ImGuiDataType_S16: *(ImS16*)(p_data) = *(ImS16*)(p_default); break;
		case ImGuiDataType_U16: *(ImU16*)(p_data) = *(ImU16*)(p_default); break;
		case ImGuiDataType_S32: *(ImS32*)(p_data) = *(ImS32*)(p_default); break;
		case ImGuiDataType_U32: *(ImU32*)(p_data) = *(ImU32*)(p_default); break;
		case ImGuiDataType_S64: *(ImS64*)(p_data) = *(ImS64*)(p_default); break;
		case ImGuiDataType_U64: *(ImU64*)(p_data) = *(ImU64*)(p_default); break;
		case ImGuiDataType_Float: *(float*)(p_data) = *(float*)(p_default); break;
		case ImGuiDataType_Double: *(double*)(p_data) = *(double*)(p_default); break;
		case ImGuiDataType_COUNT: break;
		}
		change = true;
	}
	ImGui::PopID();

	ImGui::SameLine();

	if (width > 0.0f)
	{
		width -= ImGui::GetItemRectSize().x - ImGui::GetStyle().ItemSpacing.x;
	}

	change |= SliderScalar(width, label, data_type, p_data, p_min, p_max, p_step, format, flags);

	return change;
}

bool ImGui::SliderIntDefault(float width, const char* label, int* v, int v_min, int v_max, int v_default, int v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalarDefault(width, label, ImGuiDataType_S32, v, &v_min, &v_max, &v_default , &v_step, format, flags);
}

bool ImGui::SliderUIntDefault(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_default, uint32_t v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalarDefault(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_default, &v_step, format, flags);
}

bool ImGui::SliderSizeTDefault(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_default, size_t v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalarDefault(width, label, ImGuiDataType_U32, v, &v_min, &v_max, &v_default, &v_step, format, flags);
}

bool ImGui::SliderFloatDefault(float width, const char* label, float* v, float v_min, float v_max, float v_default, float v_step, const char* format, ImGuiSliderFlags flags)
{
	return SliderScalarDefault(width, label, ImGuiDataType_Float, v, &v_min, &v_max, &v_default, &v_step, format, flags);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// COMBO ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline float inCalcMaxPopupHeightFromItemCount(int items_count)
{
	ImGuiContext& g = *GImGui;
	if (items_count <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ImGui::BeginContrastedCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
	// Always consume the SetNextWindowSizeConstraint() call in our early return paths
	ImGuiContext& g = *GImGui;
	bool has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
	g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

	const ImGuiStyle& style = g.Style;

	PushID(++ImGui::CustomStyle::pushId);
	const ImGuiID id = window->GetID(label);
	PopID();

	const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const float expected_w = CalcItemWidth();
	const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id, &frame_bb))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	bool popup_open = IsPopupOpen(id, ImGuiPopupFlags_None);

	const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
	RenderNavHighlight(frame_bb, id);
	if (!(flags & ImGuiComboFlags_NoPreview))
		window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left);
	if (!(flags & ImGuiComboFlags_NoArrowButton))
	{
		const bool pushed = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);
		ImU32 bg_col = GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImU32 text_col = GetColorU32(ImGuiCol_Text);
		window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
		if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
			RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
		if (pushed)
			ImGui::PopStyleColor();
	}
	RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
	if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
	{
		ImVec2 preview_pos = frame_bb.Min + style.FramePadding;
		if (g.LogEnabled)
			LogSetNextTextDecoration("{", "}");
		RenderTextClipped(preview_pos, ImVec2(value_x2, frame_bb.Max.y), preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
	}
	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

	if ((pressed || g.NavActivateId == id) && !popup_open)
	{
		if (window->DC.NavLayerCurrent == 0)
			window->NavLastIds[0] = id;
		OpenPopupEx(id, ImGuiPopupFlags_None);
		popup_open = true;
	}

	if (!popup_open)
		return false;

	if (has_window_size_constraint)
	{
		g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
		g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
	}
	else
	{
		if ((flags & ImGuiComboFlags_HeightMask_) == 0)
			flags |= ImGuiComboFlags_HeightRegular;
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));    // Only one
		int popup_max_height_in_items = -1;
		if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
		else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
		else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
		SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, inCalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
	}

	char name[16];
	ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

	// Position the window given a custom constraint (peak into expected window size so we can position it)
	// This might be easier to express with an hypothetical SetNextWindowPosConstraints() function.
	if (ImGuiWindow* popup_window = FindWindowByName(name))
		if (popup_window->WasActive)
		{
			// Always override 'AutoPosLastDirection' to not leave a chance for a past value to affect us.
			ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
			if (flags & ImGuiComboFlags_PopupAlignLeft)
				popup_window->AutoPosLastDirection = ImGuiDir_Left; // "Below, Toward Left"
			else
				popup_window->AutoPosLastDirection = ImGuiDir_Down; // "Below, Toward Right (default)"
			ImRect r_outer = GetWindowAllowedExtentRect(popup_window);
			ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
			SetNextWindowPos(pos);
		}

	// We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

	// Horizontally align ourselves with the framed text
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
	bool ret = Begin(name, NULL, window_flags);
	PopStyleVar();
	if (!ret)
	{
		EndPopup();
		IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
		return false;
	}
	return true;
}

bool ImGui::ContrastedCombo(float vWidth, const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
	ImGuiContext& g = *GImGui;

	if (vWidth > -1)
		ImGui::PushItemWidth((float)vWidth);

	// Call the getter to obtain the preview string which is a parameter to BeginCombo()
	const char* preview_value = NULL;
	if (*current_item >= 0 && *current_item < items_count)
		items_getter(data, *current_item, &preview_value);

	// The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
	if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
		SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, inCalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

	if (!BeginContrastedCombo(label, preview_value, ImGuiComboFlags_None))
		return false;

	// Display items
	// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
	bool value_changed = false;
	for (int i = 0; i < items_count; i++)
	{
		PushID((void*)(intptr_t)i);
		const bool item_selected = (i == *current_item);
		const char* item_text;
		if (!items_getter(data, i, &item_text))
			item_text = "*Unknown item*";
		if (Selectable(item_text, item_selected))
		{
			value_changed = true;
			*current_item = i;
		}
		if (item_selected)
			SetItemDefaultFocus();
		PopID();
	}

	EndCombo();
	if (value_changed)
		MarkItemEdited(g.CurrentWindow->DC.LastItemId);

	if (vWidth > -1)
		ImGui::PopItemWidth();

	return value_changed;
}

// Getter for the old Combo() API: const char*[]
inline bool inItems_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
inline bool inItems_SingleStringGetter(void* data, int idx, const char** out_text)
{
	// FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
	const char* items_separated_by_zeros = (const char*)data;
	int items_count = 0;
	const char* p = items_separated_by_zeros;
	while (*p)
	{
		if (idx == items_count)
			break;
		p += strlen(p) + 1;
		items_count++;
	}
	if (!*p)
		return false;
	if (out_text)
		*out_text = p;
	return true;
}

// Combo box helper allowing to pass an array of strings.
bool ImGui::ContrastedCombo(float vWidth, const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
	PushID(++ImGui::CustomStyle::pushId);

	const bool value_changed = ContrastedCombo(vWidth, label, current_item, inItems_ArrayGetter, (void*)items, items_count, height_in_items);
	
	PopID();

	return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal holding multiple zero-terminated items "item1\0item2\0"
bool ImGui::ContrastedCombo(float vWidth, const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
	PushID(++ImGui::CustomStyle::pushId);

	int items_count = 0;
	const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
	while (*p)
	{
		p += strlen(p) + 1;
		items_count++;
	}
	bool value_changed = ContrastedCombo(vWidth, label, current_item, inItems_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
	
	PopID();

	return value_changed;
}

bool ImGui::ContrastedComboVectorDefault(float vWidth, const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int vDefault, int height_in_items)
{
	bool change = false;

	if (items_count > 0)
	{
		ImGui::PushID(++CustomStyle::pushId);

		change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
		if (change)
			*current_item = vDefault;

		ImGui::SameLine();

		change |= ContrastedCombo(vWidth, label, current_item, [](void* data, int idx, const char** out_text)
			{
				*out_text = ((const std::vector<std::string>*)data)->at(idx).c_str();
				return true;
			}, (void*)&items, items_count, height_in_items);

		ImGui::PopID();
	}

	return change;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// INPUT ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMGUI_API bool ImGui::InputFloatDefault(float vWidth, const char* vName, float* vVar, float vDefault, const char* vInputPrec, const char* vPopupPrec, bool vShowResetButton, float vStep, float vStepFast)
{
	bool change = false;

	const float padding = ImGui::GetStyle().FramePadding.x;

	/*if (!ImGui::GetCurrentWindow()->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}*/

	float w = vWidth - padding * 2.0f;// -24;

	if (vShowResetButton)
	{
		change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
		w -= ImGui::GetItemRectSize().x;
		if (change)
			*vVar = vDefault;
	}

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(++CustomStyle::pushId);
	ImGui::PushItemWidth(w);
	change |= ImGui::InputFloat(vName, vVar, vStep, vStepFast, vInputPrec);
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		ImGui::SetTooltip(vPopupPrec, *vVar);

	return change;
}

bool ImGui::InputFloatDefaultStepper(float vWidth, const char* vName, float* vVar, float vDefault, float vStep, float vStepFast, const char* vInputPrec, const char* vPopupPrec, bool vShowResetButton)
{
	bool change = false;

	const float padding = ImGui::GetStyle().FramePadding.x;

	float w = vWidth - padding * 2.0f;// -24;

	if (vShowResetButton)
	{
		change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
		w -= ImGui::GetItemRectSize().x;
		if (change)
			*vVar = vDefault;
	}

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(++CustomStyle::pushId);
	ImGui::PushItemWidth(w);
	change |= ImGui::InputFloat("##InputFloat", vVar, 0.0f, 0.0f, vInputPrec, ImGuiInputTextFlags_CharsScientific);
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (vStep > 0.0f)
	{
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
		if (ImGui::ContrastedButton(ADD_BUTTON_ICON, nullptr, nullptr, 0.0f, ImVec2(0.0f, 0.0f), button_flags))
		{
			*vVar += ImGui::GetIO().KeyCtrl ? vStepFast : vStep;
		}
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		if (ImGui::ContrastedButton(REMOVE_BUTTON_ICON, nullptr, nullptr, 0.0f, ImVec2(0.0f, 0.0f), button_flags))
		{
			*vVar -= ImGui::GetIO().KeyCtrl ? vStepFast : vStep;
		}
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("%s", vName);
	}
	else
	{
		ImGui::Text("%s", vName);
	}

	if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		ImGui::SetTooltip(vPopupPrec, *vVar);

	return change;
}

IMGUI_API bool ImGui::InputIntDefault(float vWidth, const char* vName, int* vVar, int step, int step_fast, int vDefault)
{
	bool change = false;

	const float padding = ImGui::GetStyle().FramePadding.x;

	if (!ImGui::GetCurrentWindow()->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	change = ImGui::ContrastedButton(RESET_BUTTON_ICON);
	const float w = vWidth - ImGui::GetItemRectSize().x - padding * 2.0f - 24;
	if (change)
		*vVar = vDefault;

	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);

	ImGui::PushID(++CustomStyle::pushId);
	ImGui::PushItemWidth(w);
	change |= ImGui::InputInt(vName, vVar, step, step_fast);
	ImGui::PopItemWidth();
	ImGui::PopID();

	if (ImGui::IsItemActive() || ImGui::IsItemHovered())
		ImGui::SetTooltip("%i", *vVar);

	return change;
}


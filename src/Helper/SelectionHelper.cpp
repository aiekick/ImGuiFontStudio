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

#include "SelectionHelper.h"

#include <MainFrame.h>
#include <Gui/ImGuiWidgets.h>
#include <Helper/Messaging.h>
#include <Panes/Manager/LayoutManager.h>
#include <Panes/FinalFontPane.h>
#include <Panes/GeneratorPane.h>
#include <Project/ProjectFile.h>
#include <Res/CustomFont.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <cinttypes> // printf zu

static bool clickedFromLastFrame = false;

SelectionHelper::SelectionHelper() = default;
SelectionHelper::~SelectionHelper() = default;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

TemporarySelectionStruct* SelectionHelper::getSelStruct(SelectionContainerEnum vSelectionContainerEnum)
{
	if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
		return &m_TmpSelectionSrc;
	assert(SelectionContainerEnum::SELECTION_CONTAINER_Count == 2); // we must modify this if we add another case in the futur
	return &m_TmpSelectionDst;
}

bool SelectionHelper::IsGlyphSelected(
	std::shared_ptr<FontInfos> vFontInfos,
	SelectionContainerEnum vSelectionContainerEnum,
	uint32_t vCodePoint)
{
	bool res = false;

	if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
	{
		if (vFontInfos)
			res = (vFontInfos->m_SelectedGlyphs.find(vCodePoint) !=
				vFontInfos->m_SelectedGlyphs.end()); // trouv?
	}
	else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
	{
		res = (m_SelectionForOperation.find(FontInfosCodePoint(vCodePoint, vFontInfos)) !=
			m_SelectionForOperation.end()); // trouv?
	}

	return res;
}

void SelectionHelper::StartSelection(SelectionContainerEnum vSelectionContainerEnum)
{
	auto s = getSelStruct(vSelectionContainerEnum);
	s->startSelWindow = ImGui::GetCurrentWindowRead();
}

bool SelectionHelper::CanWeApplySelection(SelectionContainerEnum vSelectionContainerEnum)
{
	bool res = false;

	auto s = getSelStruct(vSelectionContainerEnum);
	size_t n = s->tmpSel.size() + s->tmpUnSel.size();
	if (n > 0)
	{
		if (ImGui::IsWindowHovered() && s->startSelWindow)
		{
			res = (s->startSelWindow == ImGui::GetHoveredWindow());
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void SelectionHelper::DrawRect(ImVec2 vPos, ImVec2 vSize)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window)
	{
		ImVec2 spa = ImGui::GetStyle().ItemSpacing;
		ImVec2 pad = ImGui::GetStyle().FramePadding * 2.0f;
		window->DrawList->AddRect(vPos - spa * 0.5f,
			vPos + vSize + pad + spa * 0.5f,
			ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 0, 15, 3.0f);
	}
}

void SelectionHelper::DrawCircle(ImVec2 vPos, float vRadius)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window)
	{
		window->DrawList->AddCircle(vPos, vRadius,
			ImGui::GetColorU32(ImGuiCol_Text), 32, 3.0f);
	}
}

void SelectionHelper::DrawLine(ImVec2 vStart, ImVec2 vEnd)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window)
	{
		window->DrawList->AddLine(
			ImVec2(vStart.x, vStart.y),
			ImVec2(vEnd.x, vEnd.y),
			ImGui::GetColorU32(ImGuiCol_Text), 3.0f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

bool SelectionHelper::IsSelectionType(GlyphSelectionTypeFlags vGlyphSelectionTypeFlags)
{
	return (m_GlyphSelectionTypeFlags & vGlyphSelectionTypeFlags);
}

bool SelectionHelper::IsSelectionMode(GlyphSelectionModeFlags vGlyphSelectionModeFlags)
{
	return (m_GlyphSelectionModeFlags & vGlyphSelectionModeFlags);
}

void SelectionHelper::DrawMenu(ProjectFile * vProjectFile)
{
	float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 3.0f;
	ImVec2 btnSize = ImVec2(maxWidth - ImGui::GetStyle().FramePadding.x, 0.0f);

	if (!m_SelectionForOperation.empty())
	{
		if (ImGui::BeginFramedGroup("Selection Operations"))
		{
			ImGui::FramedGroupText("Selection : %u Glyphs", m_SelectionForOperation.size());

			if (ImGui::Button("Remove From Final", btnSize))
			{
				RemoveSelectionFromFinal(vProjectFile);
			}

			ImGui::FramedGroupText("Re Range Code Points");

			if (!m_ReRangeStruct.startCodePoint.valid)
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.2f, 0.2f, 0.8f));
			bool edited = ImGui::SliderUIntCompact(maxWidth + ImGui::GetStyle().FramePadding.x,
				"Start CodePoint", &m_ReRangeStruct.startCodePoint.codePoint, 0U, 65535U);
			if (!m_ReRangeStruct.startCodePoint.valid)
				ImGui::PopStyleColor();
			if (edited)
			{
				m_ReRangeStruct.startCodePoint.valid =
					m_ReRangeStruct.startCodePoint.codePoint + (int)m_SelectionForOperation.size() <=
					m_ReRangeStruct.MaxCodePoint;
			}

			if (!m_ReRangeStruct.endCodePoint.valid)
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.8f, 0.2f, 0.2f, 0.8f));
			edited = ImGui::SliderUIntCompact(maxWidth + ImGui::GetStyle().FramePadding.x,
				"End CodePoint", &m_ReRangeStruct.endCodePoint.codePoint, 0U, 65535U);
			if (!m_ReRangeStruct.endCodePoint.valid)
				ImGui::PopStyleColor();
			if (edited)
			{
				m_ReRangeStruct.endCodePoint.valid =
					m_ReRangeStruct.endCodePoint.codePoint - (int)m_SelectionForOperation.size() >=
					m_ReRangeStruct.MinCodePoint;
			}

			if (m_ReRangeStruct.startCodePoint.codePoint + (int)m_SelectionForOperation.size() <=
				m_ReRangeStruct.MaxCodePoint)
			{
				if (ImGui::Button("ReRange after start", btnSize))
				{
					ReRange_Offset_After_Start(vProjectFile, (uint32_t)m_ReRangeStruct.startCodePoint.codePoint);
				}
			}

			if (m_ReRangeStruct.endCodePoint.codePoint - (int)m_SelectionForOperation.size() >=
				m_ReRangeStruct.MinCodePoint)
			{
				if (ImGui::Button("ReRange before end", btnSize))
				{
					ReRange_Offset_Before_End(vProjectFile, (uint32_t)m_ReRangeStruct.endCodePoint.codePoint);
				}
			}

			ImGui::EndFramedGroup(true);
		}
	}

	if (ImGui::BeginFramedGroup("Tools"))
	{
		ImGui::FramedGroupText("Selection Type by Mouse");

#ifdef _DEBUG 
		/*ImGui::Text("FirstClick Selected : %i", m_GlyphSelectedStateFirstClick);
		/ImGui::Text("Src Sel Count : %zu", m_TmpSelectionSrc.tmpSel.size());
		ImGui::Text("Src UnSel Count : %zu", m_TmpSelectionSrc.tmpUnSel.size());
		ImGui::Text("Dst Sel Count : %zu", m_TmpSelectionDst.tmpSel.size());
		ImGui::Text("Dst UnSel Count : %zu", m_TmpSelectionDst.tmpUnSel.size());*/
#endif

		float mrw = maxWidth / 3.0f - ImGui::GetStyle().FramePadding.x;
		ImGui::RadioButtonLabeled_BitWize<GlyphSelectionTypeFlags>("Zone", "by Zone",
			&m_GlyphSelectionTypeFlags, GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE, mrw, true);
		ImGui::SameLine();
		ImGui::RadioButtonLabeled_BitWize<GlyphSelectionTypeFlags>("Range", "by Range",
			&m_GlyphSelectionTypeFlags, GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_RANGE, mrw, true);
		ImGui::SameLine();
		ImGui::RadioButtonLabeled_BitWize<GlyphSelectionTypeFlags>("Line", "by Line",
			&m_GlyphSelectionTypeFlags, GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_LINE, mrw, true);

		if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE))
		{
			ImGui::SliderFloatDefaultCompact(maxWidth + ImGui::GetStyle().FramePadding.x,
				"Zone Radius", &m_Zone.z, 0.5f, 150.0f, m_Zone.w);
		}

		if (!IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_RANGE))
		{
			ImGui::FramedGroupText("Selection Mode");
			mrw = maxWidth / 2.0f - ImGui::GetStyle().FramePadding.x;
			ImGui::RadioButtonLabeled_BitWize<GlyphSelectionModeFlags>(
				"Add", "Select glyphs in additive mode",
				&m_GlyphSelectionModeFlags, GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_ADD, mrw, true);
			ImGui::SameLine();
			ImGui::RadioButtonLabeled_BitWize<GlyphSelectionModeFlags>(
				"Inv", "Select glyphs in inverse mode",
				&m_GlyphSelectionModeFlags, GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_INVERSE, mrw, true);
		}

		ImGui::EndFramedGroup(true);
	}
}

void SelectionHelper::DrawSelectionMenu(ProjectFile * vProjectFile, SelectionContainerEnum vSelectionContainerEnum)
{
	if (ImGui::BeginMenu("Selection"))
	{
		if (ImGui::MenuItem("Select All Glyphs", ""))
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				SelectAllGlyphs(
					vProjectFile, vProjectFile->m_SelectedFont,
					vSelectionContainerEnum);
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				for (auto& font : vProjectFile->m_Fonts)
				{
					SelectAllGlyphs(
						vProjectFile, font.second,
						vSelectionContainerEnum);
				}

				FinalizeSelectionForOperations();
			}
		}

		if (ImGui::MenuItem("UnSelect All Glyphs", ""))
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				UnSelectAllGlyphs(
					vProjectFile, vProjectFile->m_SelectedFont,
					vSelectionContainerEnum);
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				for (auto& font : vProjectFile->m_Fonts)
				{
					UnSelectAllGlyphs(
						vProjectFile, font.second,
						vSelectionContainerEnum);
				}

				FinalizeSelectionForOperations();
			}
		}

		ImGui::EndMenu();
	}
}

void SelectionHelper::SelectWithToolOrApply(
	ProjectFile * vProjectFile,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_LINE))
	{
		SelectByLine(vProjectFile, vSelectionContainerEnum);
	}
	else if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE))
	{
		SelectByZone(vProjectFile, vSelectionContainerEnum);
	}
}

void SelectionHelper::SelectWithToolOrApplyOnGlyph(
	ProjectFile * vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph,
	uint32_t vGlyphIdx,
	bool vGlypSelected,
	bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_RANGE))
	{
		if (vGlypSelected)
		{
			SelectGlyphByRangeFromStartCodePoint(
				vProjectFile, vFontInfos, vGlyph, vGlyphIdx, vUpdateMaps, vSelectionContainerEnum);
		}
		else
		{
			UnSelectGlyphByRangeFromStartCodePoint(
				vProjectFile, vFontInfos, vGlyph, vGlyphIdx, vUpdateMaps, vSelectionContainerEnum);
		}
	}
}

bool SelectionHelper::IsGlyphIntersectedAndSelected(
	std::shared_ptr<FontInfos> vFontInfos, ImVec2 vCellSize, uint32_t vCodePoint, bool* vSelected,
	SelectionContainerEnum vSelectionContainerEnum)
{
	bool intersected = false;

	if (vSelected)
	{
		if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) // prevent selection / hovered if on another window
		{
			if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_LINE))
			{
				intersected = DrawGlyphSelectionByLine(
					vFontInfos, vCellSize, vCodePoint, vSelected, vSelectionContainerEnum);
			}
			else if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE))
			{
				intersected = DrawGlyphSelectionByZone(
					vFontInfos, vCellSize, vCodePoint, vSelected, vSelectionContainerEnum);
			}
		}

		if (IsGlyphSelected(vFontInfos, vSelectionContainerEnum, vCodePoint))
		{
			if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_LINE))
			{
				if (!intersected)
					*vSelected = true;
			}
			else if (IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE))
			{
				if (!intersected)
				{
					*vSelected = true;

					if (getSelStruct(vSelectionContainerEnum)->isUnSelected(vCodePoint, vFontInfos)) // found
					{
						*vSelected = false;
					}
				}
			}
			else
			{
				*vSelected = true;
			}
		}
	}

	return intersected;
}

///////////////////////////////////////////////////////////////////////////////////////////
//////// SELECTION IF INTERSECTED WITH GLYPH///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void SelectionHelper::GlyphSelectionIfIntersected(
	std::shared_ptr<FontInfos> vFontInfos, ImVec2
#ifdef _DEBUG 
	vCaseSize
#endif
	, uint32_t vCodePoint,
	bool* vSelected,
	SelectionContainerEnum vSelectionContainerEnum)
{
	bool selected = IsGlyphSelected(vFontInfos, vSelectionContainerEnum, vCodePoint);

	// attention this item is catched at frame start
	// but the start of selection mode is at end frame so this is started for LINE at the last frame.
	// so for LINE mode, IsMouseClicked is never catched
	// ZONE is Ok because not use IsMouseClicked for start mode of ZONE
	// __clickedFromLastFrame is here for use the click of the last frame only for line mode
	// todo : need to found a better behavior
	if (ImGui::IsMouseClicked(0) || clickedFromLastFrame)
	{
		m_GlyphSelectedStateFirstClick = (selected ? 1 : 0);
		StartSelection(vSelectionContainerEnum);
		clickedFromLastFrame = false;
	}

	if (vSelected)
	{
		if (IsSelectionMode(GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_INVERSE))
		{
			*vSelected = !selected;
		}
		else if (IsSelectionMode(GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_ADD))
		{
			if (m_GlyphSelectedStateFirstClick > -1)
				*vSelected = (m_GlyphSelectedStateFirstClick == 0);
			else
				*vSelected = true;
		}

		// same issue for line mode as IsMouseClicked
		if (ImGui::IsMouseDown(0))
		{
			if (*vSelected)
			{
				// is not selected => select
				getSelStruct(vSelectionContainerEnum)->Select(vCodePoint, vFontInfos);

#ifdef _DEBUG 
				DrawRect(ImGui::GetCursorScreenPos(), vCaseSize);
#endif
			}
			else
			{
				getSelStruct(vSelectionContainerEnum)->UnSelect(vCodePoint, vFontInfos);
			}
		}
	}

	if (!ImGui::IsMouseDown(0))
	{
		m_GlyphSelectedStateFirstClick = -1;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//////// SELECTION BY LINE ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void SelectionHelper::SelectByLine(ProjectFile * vProjectFile,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (IS_FLOAT_DIFFERENT(m_Line.sumAbs(), 0.0f))
	{
		if (ImGui::IsMouseDown(0))
		{
			m_Line.z = ImGui::GetMousePos().x;
			m_Line.w = ImGui::GetMousePos().y;

			DrawLine(ImVec2(m_Line.x, m_Line.y), ImVec2(m_Line.z, m_Line.w));
		}
		else if (ImGui::IsMouseReleased(0))// catched one frame after the end of selection mode,and the map is refilled with sel datas..
		{
			ApplySelection(vProjectFile, vSelectionContainerEnum);
			m_Line.Set(0, 0, 0, 0);
		}
	}
	// found another way for start mode to use this IsMouseClicked
	// this func is caleed after the icons dipaly so at frame end
	// but the icon selection is made during the icon display so before.
	else if (ImGui::IsMouseClicked(0))
	{
		m_Line.x = ImGui::GetMousePos().x;
		m_Line.y = ImGui::GetMousePos().y;
		m_Line.z = m_Line.x;
		m_Line.w = m_Line.y;
		clickedFromLastFrame = true;
	}
}

// return true if intersected by line
bool SelectionHelper::DrawGlyphSelectionByLine(
	std::shared_ptr<FontInfos> vFontInfos,
	ImVec2 vCaseSize,
	uint32_t vCodePoint,
	bool* vSelected,
	SelectionContainerEnum vSelectionContainerEnum)
{
	bool intersected = false;

	ImVec2 startRect = ImGui::GetCursorScreenPos();
	ImVec2 pad = ImGui::GetStyle().FramePadding * 2.0f;
	ct::frect rc(startRect.x, startRect.y, vCaseSize.x + pad.x, vCaseSize.y + pad.y);
	if (rc.IsIntersectedByLine(m_Line.xy(), m_Line.zw()))
	{
		GlyphSelectionIfIntersected(
			vFontInfos, vCaseSize,
			vCodePoint, vSelected, vSelectionContainerEnum);

		intersected = true;
	}
	else
	{
		getSelStruct(vSelectionContainerEnum)->Clear(vCodePoint, vFontInfos);
	}

	return intersected;
}

///////////////////////////////////////////////////////////////////////////////////////////
//////// SELECTION BY ZONE ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void SelectionHelper::SelectByZone(
	ProjectFile * vProjectFile,
	SelectionContainerEnum vSelectionContainerEnum)
{
	m_Zone.x = ImGui::GetMousePos().x;
	m_Zone.y = ImGui::GetMousePos().y;

	DrawCircle(ImVec2(m_Zone.x, m_Zone.y), m_Zone.z);

	if (ImGui::IsMouseReleased(0))
	{
		ApplySelection(vProjectFile, vSelectionContainerEnum);
		m_Zone.Set(0, 0, m_Zone.z, m_Zone.w);
	}
}

bool SelectionHelper::DrawGlyphSelectionByZone(
	std::shared_ptr<FontInfos> vFontInfos,
	ImVec2 vCaseSize,
	uint32_t vCodePoint,
	bool* vSelected,
	SelectionContainerEnum vSelectionContainerEnum)
{
	bool intersected = false;

	ImVec2 startRect = ImGui::GetCursorScreenPos();
	ImVec2 pad = ImGui::GetStyle().FramePadding * 2.0f;
	ct::frect rc(startRect.x, startRect.y, vCaseSize.x + pad.x, vCaseSize.y + pad.y);
	if (rc.IsIntersectedByCircle(m_Zone.xy(), m_Zone.z)) // intersected
	{
		GlyphSelectionIfIntersected(
			vFontInfos, vCaseSize,
			vCodePoint, vSelected, vSelectionContainerEnum);

		intersected = true;
	}
	else // trail
	{
		if (!ImGui::IsMouseDown(0))
		{
			if (!ImGui::IsMouseReleased(0))
			{
				getSelStruct(vSelectionContainerEnum)->Clear(vCodePoint, vFontInfos);
			}
		}

		if (getSelStruct(vSelectionContainerEnum)->isSelected(vCodePoint, vFontInfos)) // found
		{
#ifdef _DEBUG 
			DrawRect(startRect, vCaseSize);
#endif
			if (vSelected)
				*vSelected = true;
		}

		if (getSelStruct(vSelectionContainerEnum)->isUnSelected(vCodePoint, vFontInfos)) // found
		{
			if (vSelected)
				*vSelected = false;
		}
	}

	return intersected;
}

void SelectionHelper::ApplySelection(ProjectFile * vProjectFile,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (CanWeApplySelection(vSelectionContainerEnum))
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				auto selStruct = getSelStruct(vSelectionContainerEnum);

				// to Select
				auto itSel = selStruct->tmpSel.begin();
				while (itSel != selStruct->tmpSel.end())
				{
					const auto codePoint = *itSel; // attention, it is a copy

					bool toErase = false;
					if (codePoint.second)
					{
						if (codePoint.second->m_ImFontAtlas.IsBuilt())
						{
							ImFont* font = codePoint.second->m_ImFontAtlas.Fonts[0];
							if (font)
							{
								auto ptr = font->FindGlyph((ImWchar)codePoint.first);
								if (ptr)
								{
									SelectGlyph(vProjectFile, codePoint.second, *ptr, false, vSelectionContainerEnum);
									toErase = true;
								}
							}
						}
					}

					if (toErase)
						itSel = selStruct->tmpSel.erase(itSel);
					else
						itSel++;
				}

				// to UnSelect
				auto itUnSel = selStruct->tmpUnSel.begin();
				while (itUnSel != selStruct->tmpUnSel.end())
				{
					const auto codePoint = *itUnSel; // attention, it is a copy

					bool toErase = false;
					if (codePoint.second)
					{
						if (codePoint.second->m_ImFontAtlas.IsBuilt())
						{
							ImFont* font = codePoint.second->m_ImFontAtlas.Fonts[0];
							if (font)
							{
								auto ptr = font->FindGlyph((ImWchar)codePoint.first);
								if (ptr)
								{
									UnSelectGlyph(vProjectFile, codePoint.second, *ptr, false, vSelectionContainerEnum);
									toErase = true;
								}
							}
						}
					}

					if (toErase)
						itUnSel = selStruct->tmpUnSel.erase(itUnSel);
					else
						itUnSel++;
				}

				// update maps
				PrepareSelection(vProjectFile, vSelectionContainerEnum);
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				auto selStruct = getSelStruct(vSelectionContainerEnum);

				// to Select
				auto itSel = selStruct->tmpSel.begin();
				while (itSel != selStruct->tmpSel.end())
				{
					const auto codePoint = *itSel; // attention, it is a copy
					SelectGlyph(vProjectFile, codePoint.second, codePoint.first, false, vSelectionContainerEnum);
					itSel = selStruct->tmpSel.erase(itSel);
				}

				// to UnSelect
				auto itUnSel = selStruct->tmpUnSel.begin();
				while (itUnSel != selStruct->tmpUnSel.end())
				{
					const auto codePoint = *itUnSel; // attention, it is a copy
					UnSelectGlyph(vProjectFile, codePoint.second, codePoint.first, false, vSelectionContainerEnum);
					itUnSel = selStruct->tmpUnSel.erase(itUnSel);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//// GLYPH SELECTION /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void SelectionHelper::SelectAllGlyphs(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				if (!vFontInfos->m_ImFontAtlas.Fonts.empty())
				{
					ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];

					if (font)
					{
						for (const auto& glyph : font->Glyphs)
						{
							SelectGlyph(vProjectFile, vFontInfos, glyph, false, vSelectionContainerEnum);
						}

						// update maps
						vProjectFile->UpdateCountSelectedGlyphs();
						PrepareSelection(vProjectFile, vSelectionContainerEnum);
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				for (const auto& glyph : vFontInfos->m_SelectedGlyphs)
				{
					SelectGlyph(vProjectFile, vFontInfos, glyph.second.glyph.Codepoint, false, vSelectionContainerEnum);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::UnSelectAllGlyphs(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				if (!vFontInfos->m_ImFontAtlas.Fonts.empty())
				{
					ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];

					if (font)
					{
						for (const auto& glyph : font->Glyphs)
						{
							UnSelectGlyph(vProjectFile, vFontInfos, glyph, false, vSelectionContainerEnum);
						}

						// update maps
						vProjectFile->UpdateCountSelectedGlyphs();
						PrepareSelection(vProjectFile, vSelectionContainerEnum);
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				for (const auto& glyph : vFontInfos->m_SelectedGlyphs)
				{
					UnSelectGlyph(vProjectFile, vFontInfos, glyph.second.glyph.Codepoint, false, vSelectionContainerEnum);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::SelectGlyph(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos, ImFontGlyph vGlyph, bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				if (vFontInfos->m_SelectedGlyphs.find(vGlyph.Codepoint) == vFontInfos->m_SelectedGlyphs.end()) // not found
				{
					std::string res = vFontInfos->GetGlyphName(vGlyph.Codepoint);
					if (res.empty())
						res = "Symbol Name";
					vFontInfos->m_SelectedGlyphs[vGlyph.Codepoint] = GlyphInfos(vGlyph, res, res);
					vProjectFile->SetProjectChange();

					if (vUpdateMaps)
					{
						vProjectFile->UpdateCountSelectedGlyphs();
						PrepareSelection(vProjectFile, vSelectionContainerEnum);
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				SelectGlyph(vProjectFile, vFontInfos, vGlyph.Codepoint, vUpdateMaps,
					vSelectionContainerEnum);

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::SelectGlyph(ProjectFile * vProjectFile, FontInfosCodePoint vFontInfosCodePoint, bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	SelectGlyph(vProjectFile, vFontInfosCodePoint.second,
		vFontInfosCodePoint.first, vUpdateMaps, vSelectionContainerEnum);
}

void SelectionHelper::SelectGlyph(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos, uint32_t vCodePoint, bool /*vUpdateMaps*/,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				assert(0);
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				auto c = FontInfosCodePoint(vCodePoint, vFontInfos);
				if (m_SelectionForOperation.find(c) == m_SelectionForOperation.end()) // not found
				{
					m_SelectionForOperation.emplace(c);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::UnSelectGlyph(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos, ImFontGlyph vGlyph, bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	UnSelectGlyph(vProjectFile, vFontInfos,
		vGlyph.Codepoint, vUpdateMaps, vSelectionContainerEnum);
}

void SelectionHelper::UnSelectGlyph(ProjectFile * vProjectFile, FontInfosCodePoint vFontInfosCodePoint, bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	UnSelectGlyph(vProjectFile, vFontInfosCodePoint.second,
		vFontInfosCodePoint.first, vUpdateMaps, vSelectionContainerEnum);
}

void SelectionHelper::UnSelectGlyph(ProjectFile * vProjectFile, std::shared_ptr<FontInfos> vFontInfos, uint32_t vCodePoint, bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				if (vFontInfos->m_SelectedGlyphs.find(vCodePoint) != vFontInfos->m_SelectedGlyphs.end()) // found
				{
					vFontInfos->m_SelectedGlyphs.erase(vCodePoint);
					vProjectFile->SetProjectChange();

					if (vUpdateMaps)
					{
						vProjectFile->UpdateCountSelectedGlyphs();
						PrepareSelection(vProjectFile, vSelectionContainerEnum);
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				auto c = FontInfosCodePoint(vCodePoint, vFontInfos);
				if (m_SelectionForOperation.find(c) != m_SelectionForOperation.end()) // found
				{
					m_SelectionForOperation.erase(c);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::RemoveSelectionFromFinal(ProjectFile * vProjectFile)
{
	for (auto& codePoint : m_SelectionForOperation)
	{
		UnSelectGlyph(vProjectFile, codePoint, false, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
	}

	PrepareSelection(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

	m_SelectionForOperation.clear();
}

void SelectionHelper::ReRange_Offset_After_Start(ProjectFile * vProjectFile, uint32_t vOffsetCodePoint)
{
	std::set<uint32_t> codePoints;
	for (auto& font : vProjectFile->m_Fonts)
	{
		for (auto& selection : font.second->m_SelectedGlyphs)
		{
			codePoints.emplace(selection.first);
		}
	}

	uint32_t pos = vOffsetCodePoint;
	for (auto& codePoint : m_SelectionForOperation)
	{
		auto fontInfos = codePoint.second;
		if (fontInfos)
		{
			while (codePoints.find(pos) != codePoints.end())
				pos++;

			if (codePoints.find(pos) == codePoints.end()) // not found
			{
				if (fontInfos->m_SelectedGlyphs.find(codePoint.first) != fontInfos->m_SelectedGlyphs.end()) // found
				{
					if (pos >= m_ReRangeStruct.MinCodePoint && pos <= m_ReRangeStruct.MaxCodePoint)
					{
						fontInfos->m_SelectedGlyphs[codePoint.first].newCodePoint = pos;
						pos++;
						vProjectFile->SetProjectChange();
					}
				}
				else
				{
					assert(0); // pas normal d'arriver la
				}
			}
		}
	}

	PrepareSelection(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
}

void SelectionHelper::ReRange_Offset_Before_End(ProjectFile * vProjectFile, uint32_t vOffsetCodePoint)
{
	std::set<uint32_t> codePoints;
	for (auto& font : vProjectFile->m_Fonts)
	{
		for (auto& selection : font.second->m_SelectedGlyphs)
		{
			codePoints.emplace(selection.first);
		}
	}

	uint32_t pos = vOffsetCodePoint;
	for (auto& codePoint : m_SelectionForOperation)
	{
		auto fontInfos = codePoint.second;
		if (fontInfos)
		{
			while (codePoints.find(pos) != codePoints.end())
				pos--;

			if (codePoints.find(pos) == codePoints.end()) // not found
			{
				if (fontInfos->m_SelectedGlyphs.find(codePoint.first) != fontInfos->m_SelectedGlyphs.end()) // found
				{
					if (pos >= m_ReRangeStruct.MinCodePoint && pos <= m_ReRangeStruct.MaxCodePoint)
					{
						fontInfos->m_SelectedGlyphs[codePoint.first].newCodePoint = pos;
						pos--;
						vProjectFile->SetProjectChange();
					}
				}
				else
				{
					assert(0); // pas normal d'arriver la
				}
			}
		}
	}

	PrepareSelection(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_FINAL);
}

void SelectionHelper::SelectGlyphByRangeFromStartCodePoint(
	ProjectFile * vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph,
	uint32_t vFontGlyphIndex,
	bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				// on va explorer font->Glyphs
				// et on va partir dans les deux sens en meme temps jusqu'a ce que le codepoint ne suivent plus une sutie logique
				// ou que les limite de m_SelectedGlyphs soit trouv?e

				if (vFontInfos->m_ImFontAtlas.IsBuilt())
				{
					ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];
					if (font)
					{
						int idx = vFontGlyphIndex;
						ImFontGlyph currentGlyph = font->Glyphs[idx];
						SelectGlyph(vProjectFile, vFontInfos, currentGlyph, false, vSelectionContainerEnum);

						int leftIdx = idx;
						int rightIdx = idx;
						uint32_t leftCodePoint = currentGlyph.Codepoint;
						uint32_t rightCodePoint = currentGlyph.Codepoint;
						while (leftIdx != -1 || rightIdx != -1)
						{
							if (leftIdx != -1)
							{
								if (leftIdx > 0) leftIdx--;
								else leftIdx = -1;
								ImFontGlyph leftGlyph = font->Glyphs[leftIdx];
								if (leftGlyph.Codepoint != leftCodePoint - 1)
								{
									// limite de range => on stop
									leftIdx = -1;
								}
								else
								{
									SelectGlyph(vProjectFile, vFontInfos, leftGlyph, false, vSelectionContainerEnum);
									leftCodePoint = leftGlyph.Codepoint;
								}
							}

							if (rightIdx != -1)
							{
								if (rightIdx < font->Glyphs.size() - 1) rightIdx++;
								else rightIdx = -1;
								ImFontGlyph rightGlyph = font->Glyphs[rightIdx];
								if (rightGlyph.Codepoint != rightCodePoint + 1)
								{
									// limite de range => on stop
									rightIdx = -1;
								}
								else
								{
									SelectGlyph(vProjectFile, vFontInfos, rightGlyph, false, vSelectionContainerEnum);
									rightCodePoint = rightGlyph.Codepoint;
								}
							}
						}

						if (vUpdateMaps)
						{
							PrepareSelection(vProjectFile, vSelectionContainerEnum);
						}
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				if (vFontInfos->m_SelectedGlyphs.find(vGlyph.Codepoint) != vFontInfos->m_SelectedGlyphs.end())
				{
					SelectGlyph(vProjectFile, vFontInfos, vGlyph.Codepoint, false, vSelectionContainerEnum);

					auto glyphs = &vFontInfos->m_SelectedGlyphs;
					auto leftIdx = glyphs->find(vGlyph.Codepoint);
					auto rightIdx = glyphs->find(vGlyph.Codepoint);

					uint32_t leftCodePoint = vGlyph.Codepoint;
					uint32_t rightCodePoint = vGlyph.Codepoint;
					while (leftIdx != glyphs->end() || rightIdx != glyphs->end())
					{
						if (leftIdx != glyphs->end())
						{
							if (leftIdx != glyphs->begin()) leftIdx--;
							else leftIdx = glyphs->end();

							if (leftIdx != glyphs->end())
							{
								if (leftIdx->second.newCodePoint != leftCodePoint - 1)
								{
									// limite de range => on stop
									leftIdx = glyphs->end();
								}
								else
								{
									SelectGlyph(vProjectFile, vFontInfos, leftIdx->second.newCodePoint, false, vSelectionContainerEnum);
									leftCodePoint = leftIdx->second.newCodePoint;
								}
							}
						}

						if (rightIdx != glyphs->end())
						{
							if (rightIdx != (--glyphs->end())) rightIdx++;
							else rightIdx = glyphs->end();

							if (rightIdx != glyphs->end())
							{
								if (rightIdx->second.newCodePoint != rightCodePoint + 1)
								{
									// limite de range => on stop
									rightIdx = glyphs->end();
								}
								else
								{
									SelectGlyph(vProjectFile, vFontInfos, rightIdx->second.newCodePoint, false, vSelectionContainerEnum);
									rightCodePoint = rightIdx->second.newCodePoint;
								}
							}
						}
					}
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::UnSelectGlyphByRangeFromStartCodePoint(
	ProjectFile * vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph,
	bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				// on va explorer font->Glyphs
				// et on va partir dans les deux sens en meme temps jusqu'a ce que le codepoint ne suivent plus une sutie logique
				// ou que les limite de m_SelectedGlyphs soit trouv?e

				if (vFontInfos->m_ImFontAtlas.IsBuilt())
				{
					ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];
					if (font)
					{
						int glyphIndex = 0;
						auto ptr = font->FindGlyph(vGlyph.Codepoint);
						if (ptr)
						{
							glyphIndex = font->Glyphs.index_from_ptr(ptr);
							UnSelectGlyphByRangeFromStartCodePoint(vProjectFile, vFontInfos,
								*ptr, glyphIndex, vUpdateMaps, vSelectionContainerEnum);
						}
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				// on va explorer font->Glyphs
				// et on va partir dans les deux sens en meme temps jusqu'a ce que le codepoint ne suivent plus une sutie logique
				// ou que les limite de m_SelectedGlyphs soit trouv?e

				if (vFontInfos->m_ImFontAtlas.IsBuilt())
				{
					UnSelectGlyphByRangeFromStartCodePoint(vProjectFile, vFontInfos,
						vGlyph, 0, vUpdateMaps, vSelectionContainerEnum);
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::UnSelectGlyphByRangeFromStartCodePoint(
	ProjectFile * vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	ImFontGlyph vGlyph,
	uint32_t vFontGlyphIndex,
	bool vUpdateMaps,
	SelectionContainerEnum vSelectionContainerEnum)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (vFontInfos)
		{
			if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_SOURCE)
			{
				// on va explorer font->Glyphs
				// et on va partir dans les deux sens en meme temps jusqu'a ce que le codepoint ne suivent plus une sutie logique
				// ou que les limite de m_SelectedGlyphs soit trouv?e

				if (vFontInfos->m_ImFontAtlas.IsBuilt())
				{
					ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];
					if (font)
					{
						int idx = vFontGlyphIndex;
						ImFontGlyph currentGlyph = font->Glyphs[idx];
						UnSelectGlyph(vProjectFile, vFontInfos, currentGlyph, false, vSelectionContainerEnum);

						int leftIdx = idx;
						int rightIdx = idx;
						uint32_t leftCodePoint = currentGlyph.Codepoint;
						uint32_t rightCodePoint = currentGlyph.Codepoint;
						while (leftIdx != -1 || rightIdx != -1)
						{
							if (leftIdx != -1)
							{
								if (leftIdx > 0) leftIdx--;
								else leftIdx = -1;
								ImFontGlyph leftGlyph = font->Glyphs[leftIdx];
								if (leftGlyph.Codepoint != leftCodePoint - 1)
								{
									// limite de range => on stop
									leftIdx = -1;
								}
								else
								{
									UnSelectGlyph(vProjectFile, vFontInfos, leftGlyph, false, vSelectionContainerEnum);
									leftCodePoint = leftGlyph.Codepoint;
								}
							}

							if (rightIdx != -1)
							{
								if (rightIdx < font->Glyphs.size() - 1) rightIdx++;
								else rightIdx = -1;
								ImFontGlyph rightGlyph = font->Glyphs[rightIdx];
								if (rightGlyph.Codepoint != rightCodePoint + 1)
								{
									// limite de range => on stop
									rightIdx = -1;
								}
								else
								{
									UnSelectGlyph(vProjectFile, vFontInfos, rightGlyph, false, vSelectionContainerEnum);
									rightCodePoint = rightGlyph.Codepoint;
								}
							}
						}

						if (vUpdateMaps)
						{
							PrepareSelection(vProjectFile, vSelectionContainerEnum);
						}
					}
				}
			}
			else if (vSelectionContainerEnum == SelectionContainerEnum::SELECTION_CONTAINER_FINAL)
			{
				if (vFontInfos->m_SelectedGlyphs.find(vGlyph.Codepoint) != vFontInfos->m_SelectedGlyphs.end())
				{
					UnSelectGlyph(vProjectFile, vFontInfos, vGlyph.Codepoint, false, vSelectionContainerEnum);

					auto glyphs = &vFontInfos->m_SelectedGlyphs;
					auto leftIdx = glyphs->find(vGlyph.Codepoint);
					auto rightIdx = glyphs->find(vGlyph.Codepoint);

					uint32_t leftCodePoint = vGlyph.Codepoint;
					uint32_t rightCodePoint = vGlyph.Codepoint;
					while (leftIdx != glyphs->end() || rightIdx != glyphs->end())
					{
						if (leftIdx != glyphs->end())
						{
							if (leftIdx != glyphs->begin()) leftIdx--;
							else leftIdx = glyphs->end();

							if (leftIdx != glyphs->end())
							{
								if (leftIdx->second.newCodePoint != leftCodePoint - 1)
								{
									// limite de range => on stop
									leftIdx = glyphs->end();
								}
								else
								{
									UnSelectGlyph(vProjectFile, vFontInfos, leftIdx->second.newCodePoint, false, vSelectionContainerEnum);
									leftCodePoint = leftIdx->second.newCodePoint;
								}
							}
						}

						if (rightIdx != glyphs->end())
						{
							if (rightIdx != (--glyphs->end())) rightIdx++;
							else rightIdx = glyphs->end();

							if (rightIdx != glyphs->end())
							{
								if (rightIdx->second.newCodePoint != rightCodePoint + 1)
								{
									// limite de range => on stop
									rightIdx = glyphs->end();
								}
								else
								{
									UnSelectGlyph(vProjectFile, vFontInfos, rightIdx->second.newCodePoint, false, vSelectionContainerEnum);
									rightCodePoint = rightIdx->second.newCodePoint;
								}
							}
						}
					}
				}

				FinalizeSelectionForOperations();
			}
		}
	}
}

void SelectionHelper::PrepareSelection(
	ProjectFile * vProjectFile,
	SelectionContainerEnum /*vSelectionContainerEnum*/)
{
	FinalFontPane::Instance()->PrepareSelection(vProjectFile);

	AnalyseSourceSelection(vProjectFile);
}

void SelectionHelper::AnalyseSourceSelection(ProjectFile * vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		Messaging::Instance()->ClearErrors();

		// search for codepoint and names in double => an generate an error
		std::set<std::string> namesGlobal; vProjectFile->m_NameFoundInDouble = false;
		std::set<uint32_t> codePointsGlobal; vProjectFile->m_CodePointFoundInDouble = false;
		std::set<std::string> namesLocal;
		std::set<uint32_t> codePointsLocal;
		for (auto& font : vProjectFile->m_Fonts)
		{
			font.second->m_NameInDoubleFound = false;
			font.second->m_CodePointInDoubleFound = false;
			namesLocal.clear();
			codePointsLocal.clear();

			for (auto& selection : font.second->m_SelectedGlyphs)
			{
				// local in current font
				if (!font.second->m_CodePointInDoubleFound)
				{
					if (codePointsLocal.find(selection.second.newCodePoint) == codePointsLocal.end()) // not found 
					{
						codePointsLocal.emplace(selection.second.newCodePoint);
					}
					else
					{
						// double detected => cast an error
						font.second->m_CodePointInDoubleFound = true;
					}
				}

				// local in current font
				if (!font.second->m_NameInDoubleFound)
				{
					if (namesLocal.find(selection.second.newHeaderName) == namesLocal.end()) // not found 
					{
						namesLocal.emplace(selection.second.newHeaderName);
					}
					else
					{
						// double detected => cast an error
						font.second->m_NameInDoubleFound = true;
					}
				}

				// global for all fonts
				if (!vProjectFile->m_CodePointFoundInDouble)
				{
					if (codePointsGlobal.find(selection.second.newCodePoint) == codePointsGlobal.end()) // not found 
					{
						codePointsGlobal.emplace(selection.second.newCodePoint);
					}
					else
					{
						// double detected => cast an error
						vProjectFile->m_CodePointFoundInDouble = true;
					}
				}

				// global for all fonts
				if (!vProjectFile->m_NameFoundInDouble)
				{
					if (namesGlobal.find(selection.second.newHeaderName) == namesGlobal.end()) // not found 
					{
						namesGlobal.emplace(selection.second.newHeaderName);
					}
					else
					{
						// double detected => cast an error
						vProjectFile->m_NameFoundInDouble = true;
					}
				}

				if (vProjectFile->m_CodePointFoundInDouble &&
					vProjectFile->m_NameFoundInDouble &&
					font.second->m_CodePointInDoubleFound &&
					font.second->m_NameInDoubleFound)
					break;
			}
		}

		if (vProjectFile->m_NameFoundInDouble)
		{
			Messaging::Instance()->AddError(true, nullptr, [this](MessageData)
				{
					ImGui::SetWindowFocus(FINAL_PANE);
					FinalFontPane::Instance()->SetFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES);
					FinalFontPane::Instance()->PrepareSelection(MainFrame::Instance()->GetProject());
				}, "Glyph names found in double ! Name export and header generation is prohibited until solved.");
			GeneratorPane::Instance()->ProhibitStatus(GeneratorStatusFlags::GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED);
		}
		else
		{
			GeneratorPane::Instance()->AllowStatus(GeneratorStatusFlags::GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED);
		}

		if (vProjectFile->m_CodePointFoundInDouble)
		{
			Messaging::Instance()->AddError(true, nullptr, [this](MessageData)
				{
					ImGui::SetWindowFocus(FINAL_PANE);
					FinalFontPane::Instance()->SetFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT);
					FinalFontPane::Instance()->PrepareSelection(MainFrame::Instance()->GetProject());
				}, "Glyph codePoint found in double ! Font merge is prohibited until solved.");
			GeneratorPane::Instance()->ProhibitStatus(GeneratorStatusFlags::GENERATOR_STATUS_FONT_MERGE_ALLOWED);
		}
		else
		{
			GeneratorPane::Instance()->AllowStatus(GeneratorStatusFlags::GENERATOR_STATUS_FONT_MERGE_ALLOWED);
		}

		for (auto& font : vProjectFile->m_Fonts)
		{
			if (font.second->m_CodePointInDoubleFound)
			{
				Messaging::Instance()->AddError(true, font.second, [this](MessageData vDatas)
					{
						ImGui::SetWindowFocus(FINAL_PANE);
						MainFrame::Instance()->GetProject()->m_SelectedFont = vDatas.GetUserDatas<FontInfos>();
						FinalFontPane::Instance()->SetFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT);
						FinalFontPane::Instance()->PrepareSelection(MainFrame::Instance()->GetProject());
					}, "Glyph codePoint found in double in font %s ! Font generation solo is prohibited until solved.", font.second->m_FontFileName.c_str());
			}

			if (font.second->m_NameInDoubleFound)
			{
				Messaging::Instance()->AddError(true, font.second, [this](MessageData vDatas)
					{
						ImGui::SetWindowFocus(FINAL_PANE);
						MainFrame::Instance()->GetProject()->m_SelectedFont = vDatas.GetUserDatas<FontInfos>();
						FinalFontPane::Instance()->SetFinalFontPaneMode(FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES);
						FinalFontPane::Instance()->PrepareSelection(MainFrame::Instance()->GetProject());
					}, "Glyph name found in double in font %s ! Font generation solo is prohibited until solved.", font.second->m_FontFileName.c_str());
			}
		}
	}
}

void SelectionHelper::FinalizeSelectionForOperations()
{
	// Prepare re range min/max
	uint32_t inf = 65535, sup = 0;
	for (auto& it : m_SelectionForOperation)
	{
		inf = ct::mini<uint32_t>(inf, it.first);
		sup = ct::maxi<uint32_t>(sup, it.first);
	}
	m_ReRangeStruct.startCodePoint.codePoint = inf;
	m_ReRangeStruct.endCodePoint.codePoint = sup;
}
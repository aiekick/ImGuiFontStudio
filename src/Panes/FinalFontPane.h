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
#pragma once

#include <Panes/Abstract/AbstractPane.h>
#include <ctools/ConfigAbstract.h>
#include<Gui/ImWidgets.h>

#include <imgui/imgui.h>
#include <map>
#include <string>
#include <vector>

enum FinalFontPaneModeFlags
{
	FINAL_FONT_PANE_NONE = 0,
	FINAL_FONT_PANE_BY_FONT_NO_ORDER = (1 << 0),
	FINAL_FONT_PANE_BY_FONT_ORDERED_BY_CODEPOINT = (1 << 1),
	FINAL_FONT_PANE_BY_FONT_ORDERED_BY_NAMES = (1 << 2),
	FINAL_FONT_PANE_MERGED_NO_ORDER = (1 << 3),
	FINAL_FONT_PANE_MERGED_ORDERED_BY_CODEPOINT = (1 << 4),
	FINAL_FONT_PANE_MERGED_ORDERED_BY_NAMES = (1 << 5)
};

class GlyphInfos;
class ProjectFile;
class FontInfos;
class FinalFontPane : public AbstractPane
{
private:
	std::vector<std::shared_ptr<GlyphInfos>> m_GlyphsMergedNoOrder;
	std::map<uint32_t, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsMergedOrderedByCodePoints;
	std::map<std::string, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsMergedOrderedByGlyphName;

private:
	FinalFontPaneModeFlags m_FinalFontPaneModeFlags = 
		FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER;
	
	bool m_GlyphEdition = false;
	bool m_AutoUpdateCodepoint_WhenEditWithButtons = false;

	ImVec4 m_GlyphButtonStateColor[3] = { ImVec4(), ImVec4(), ImVec4() };

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown)  override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)  override;

	// Preparation
	void SetFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags);
	bool IsFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags);
	void PrepareSelection();

private:
	void DrawFinalFontPane(PaneFlags& vInOutPaneShown);

	bool DrawGlyph(
		std::shared_ptr<FontInfos> vFontInfos, const ImVec2& vSize,
		std::shared_ptr<GlyphInfos> vGlyph, bool vShowRect,
		ImVec4 vGlyphButtonStateColor[3],
		bool *vNameupdated, bool *vCodePointUpdated,
		bool vForceEditMode = false);
	
	void DrawSelectionsByFontNoOrder(
		bool vShowTooltipInfos = false);
	void DrawSelectionsByFontNoOrder_OneFontOnly(
		std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

public:
	static void PrepareSelectionByFontOrderedByCodePoint();
	void DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(
		std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

private:
	void DrawSelectionsByFontOrderedByCodePoint(
		bool vShowTooltipInfos = false);

public:
	static void PrepareSelectionByFontOrderedByGlyphNames();
	void DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(
		std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

private:
	void DrawSelectionsByFontOrderedByGlyphNames(
		bool vShowTooltipInfos = false);

	void PrepareSelectionMergedNoOrder();
	void DrawSelectionMergedNoOrder();

	void PrepareSelectionMergedOrderedByCodePoint();
	void DrawSelectionMergedOrderedByCodePoint();

	void PrepareSelectionMergedOrderedByGlyphNames();
	void DrawSelectionMergedOrderedByGlyphNames();

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static std::shared_ptr<FinalFontPane> Instance()
	{
		static auto _instance = std::make_shared<FinalFontPane>();
		return _instance;
	}

public:
	FinalFontPane(); // Prevent construction
	FinalFontPane(const FinalFontPane&) {}; // Prevent construction by copying
	FinalFontPane& operator =(const FinalFontPane&) { return *this; }; // Prevent assignment
	~FinalFontPane(); // Prevent unwanted destruction};
};


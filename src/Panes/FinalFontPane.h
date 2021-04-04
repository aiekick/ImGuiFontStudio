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
#include <Gui/ImWidgets.h>

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

enum SelectedFontPaneModeFlags
{
	SELECTED_FONT_PANE_NONE = 0,
	SELECTED_FONT_PANE_ORDERED_BY_CODEPOINT = (1 << 0),
	SELECTED_FONT_PANE_ORDERED_BY_NAMES = (1 << 1),
};

class GlyphInfos;
class ProjectFile;
class FontInfos;
class FinalFontPane : public AbstractPane
{
private: // per pane settings to save
	//int m_Final_GlyphSize_Policy_Count = 20;
	//float m_Final_GlyphSize_Policy_Width = 40.0f;
	//int m_Selected_GlyphSize_Policy_Count = 20;
	//float m_Selected_GlyphSize_Policy_Width = 40.0f;

private:
	std::vector<std::shared_ptr<GlyphInfos>> m_GlyphsMergedNoOrder;
	std::map<uint32_t, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsMergedOrderedByCodePoints;
	std::map<std::string, std::vector<std::shared_ptr<GlyphInfos>>> m_GlyphsMergedOrderedByGlyphName;

private:
	FinalFontPaneModeFlags m_FinalFontPaneModeFlags = 
		FinalFontPaneModeFlags::FINAL_FONT_PANE_BY_FONT_NO_ORDER;
	SelectedFontPaneModeFlags m_SelectedFontPaneModeFlags =
		SelectedFontPaneModeFlags::SELECTED_FONT_PANE_ORDERED_BY_NAMES;

	bool m_GlyphEdition = false;
	bool m_AutoUpdateCodepoint_WhenEditWithButtons = false;

public:
	void Init() override;
	void Unit() override;
	int DrawPanes(ProjectFile* vProjectFile, int vWidgetId) override;
	void DrawDialogsAndPopups(ProjectFile* vProjectFile) override;
	int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas) override;

	// Preparation
	void SetFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags);
	bool IsFinalFontPaneMode(FinalFontPaneModeFlags vFinalFontPaneModeFlags);
	bool IsSelectedFontPaneMode(SelectedFontPaneModeFlags vSelectedFontPaneModeFlags);
	void PrepareSelection(ProjectFile *vProjectFile);

private:
	void DrawFinalFontPane(ProjectFile* vProjectFile);
	void DrawSelectedFontPane(ProjectFile* vProjectFile);

	bool DrawGlyph(ProjectFile *vProjectFile, 
		std::shared_ptr<FontInfos> vFontInfos, const ImVec2& vSize,
		std::shared_ptr<GlyphInfos> vGlyph, bool vShowRect,
		bool *vNameupdated, bool *vCodePointUpdated,
		bool vForceEditMode = false);
	
	void DrawSelectionsByFontNoOrder(ProjectFile *vProjectFile,
		bool vShowTooltipInfos = false);
	void DrawSelectionsByFontNoOrder_OneFontOnly(
		ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

	static void PrepareSelectionByFontOrderedByCodePoint(ProjectFile *vProjectFile);
	void DrawSelectionsByFontOrderedByCodePoint(ProjectFile *vProjectFile,
		bool vShowTooltipInfos = false);
	void DrawSelectionsByFontOrderedByCodePoint_OneFontOnly(
		ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

	static void PrepareSelectionByFontOrderedByGlyphNames(ProjectFile *vProjectFile);
	void DrawSelectionsByFontOrderedByGlyphNames(ProjectFile *vProjectFile,
		bool vShowTooltipInfos = false);
	void DrawSelectionsByFontOrderedByGlyphNames_OneFontOnly(
		ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		bool vWithFramedGroup = true, 
		bool vForceEditMode = false, 
		bool vForceEditModeOneColumn = false,
		bool vShowTooltipInfos = false);

	void PrepareSelectionMergedNoOrder(ProjectFile *vProjectFile);
	void DrawSelectionMergedNoOrder(ProjectFile *vProjectFile);

	void PrepareSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile);
	void DrawSelectionMergedOrderedByCodePoint(ProjectFile *vProjectFile);

	void PrepareSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile);
	void DrawSelectionMergedOrderedByGlyphNames(ProjectFile *vProjectFile);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public: // singleton
	static FinalFontPane *Instance()
	{
		static FinalFontPane *_instance = new FinalFontPane();
		return _instance;
	}

protected:
	FinalFontPane(); // Prevent construction
	FinalFontPane(const FinalFontPane&) {}; // Prevent construction by copying
	FinalFontPane& operator =(const FinalFontPane&) { return *this; }; // Prevent assignment
	~FinalFontPane(); // Prevent unwanted destruction};
};


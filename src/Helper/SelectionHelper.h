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

#include <ctools/cTools.h>

#include <string>
#include <set>
#include <memory>

enum GlyphSelectionTypeFlags
{
	GLYPH_SELECTION_TYPE_NONE = 0,
	GLYPH_SELECTION_TYPE_BY_ZONE = (1 << 4),
	GLYPH_SELECTION_TYPE_BY_RANGE = (1 << 2),
	GLYPH_SELECTION_TYPE_BY_LINE = (1 << 3),
};

enum GlyphSelectionModeFlags
{
	GLYPH_SELECTION_MODE_NONE = 0,
	GLYPH_SELECTION_MODE_ADD = (1 << 1),
	GLYPH_SELECTION_MODE_INVERSE = (1 << 2),
};

enum SelectionContainerEnum
{
	SELECTION_CONTAINER_SOURCE = 0,
	SELECTION_CONTAINER_FINAL,
	SELECTION_CONTAINER_Count
};

struct ReRangeLimitStruct
{
	bool valid = true; // for imgui color
    uint32_t codePoint = 0;
};

struct ReRangeStruct
{
	ReRangeLimitStruct startCodePoint;
	ReRangeLimitStruct endCodePoint;
	const uint32_t MinCodePoint = 0;
	const uint32_t MaxCodePoint = 65535;
};

class FontInfos;
struct ImGuiWindow;
typedef std::pair<uint32_t, std::shared_ptr<FontInfos>> FontInfosCodePoint;
struct TemporarySelectionStruct
{
	// for avoid selection apply if seletion ended outside of start window
	ImGuiWindow *startSelWindow = 0;

	std::set<FontInfosCodePoint> tmpSel;
	std::set<FontInfosCodePoint> tmpUnSel;

	bool isSelected(uint32_t c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosCodePoint(c, f);
		return (tmpSel.find(p) != tmpSel.end()); // found
	}
	
	bool isUnSelected(uint32_t c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosCodePoint(c, f);
		return (tmpUnSel.find(p) != tmpUnSel.end()); // found
	}
	void Select(uint32_t c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosCodePoint(c, f);
		tmpSel.emplace(p);
		tmpUnSel.erase(p);
	}
	void UnSelect(uint32_t c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosCodePoint(c, f);
		tmpSel.erase(p);
		tmpUnSel.emplace(p);
	}
	void Clear(uint32_t c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosCodePoint(c, f);
		tmpSel.erase(p);
		tmpUnSel.erase(p);
	}
	void Clear()
	{
		tmpSel.clear();
		tmpUnSel.clear();
	}
};

class ProjectFile;
class SelectionHelper
{
private: // Vars
	GlyphSelectionTypeFlags m_GlyphSelectionTypeFlags =	GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE;
	GlyphSelectionModeFlags m_GlyphSelectionModeFlags =	GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_ADD;
	//start x,y, end z,w // if not active x,y,z,w == 0.0
	ct::fvec4 m_Line;
	// pos x,y, radius z, default radius // if not active x,y == 0,0
	ct::fvec4 m_Zone = ct::fvec4(0.0f, 0.0f, 0.5f, 0.5f);
	// selection for operations in final pane
	std::set<FontInfosCodePoint> m_SelectionForOperation;
	// first glyph state when clicked
	// if not selected will apply unseletion
	// if selected will apply selection
	int m_GlyphSelectedStateFirstClick = -1;

private: // structs / classes
	ReRangeStruct m_ReRangeStruct; // re range : change range of selection
	TemporarySelectionStruct m_TmpSelectionSrc; // for selection of glyphs from sources
	TemporarySelectionStruct m_TmpSelectionDst; // for operations on final seletected glyphs // like re range by glyph group

private:
	TemporarySelectionStruct* getSelStruct(SelectionContainerEnum vSelectionContainerEnum);
	bool IsGlyphSelected(
		std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum,
		uint32_t vCodePoint);
	void StartSelection(SelectionContainerEnum vSelectionContainerEnum);
	bool CanWeApplySelection(SelectionContainerEnum vSelectionContainerEnum);
	
private:
	static void DrawRect(ImVec2 vPos, ImVec2 vSize);
	static void DrawCircle(ImVec2 vPos, float vRadius);
	static void DrawLine(ImVec2 vStart, ImVec2 vEnd);

public:
	void DrawMenu(ProjectFile *vProjectFile);
	void DrawSelectionMenu(ProjectFile *vProjectFile, SelectionContainerEnum vSelectionContainerEnum);
	
public:
	std::set<FontInfosCodePoint>* GetSelection();
public:
	void SelectWithToolOrApply(
		ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectWithToolOrApplyOnGlyph(
		ProjectFile *vProjectFile, 
		std::shared_ptr<FontInfos> vFontInfos, 
		ImFontGlyph vGlyph,
        uint32_t vGlyphIdx,
		bool vGlypSelected,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	bool IsGlyphIntersectedAndSelected(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCellSize, 
		uint32_t vCodePoint,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	bool IsSelectionMode(GlyphSelectionModeFlags vGlyphSelectionModeFlags);
	bool IsSelectionType(GlyphSelectionTypeFlags vGlyphSelectionTypeFlags);
	void AnalyseSourceSelection(ProjectFile *vProjectFile);

private:
	void SelectAllGlyphs(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, ImFontGlyph vGlyph, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, FontInfosCodePoint vFontInfosCodePoint, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, uint32_t vCodePoint, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	
	void UnSelectAllGlyphs(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, ImFontGlyph vGlyph, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, FontInfosCodePoint vFontInfosCodePoint, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, uint32_t vCodePoint, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);

private:
	void RemoveSelectionFromFinal(ProjectFile *vProjectFile);
	void ReRange_Offset_After_Start(ProjectFile *vProjectFile, uint32_t vOffsetCodePoint);
	void ReRange_Offset_Before_End(ProjectFile *vProjectFile, uint32_t vOffsetCodePoint);
	
private: // selection for view modes
	void PrepareSelection(ProjectFile *vProjectFile,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // ReRange
	void FinalizeSelectionForOperations();

private: // selections mode common
	void GlyphSelectionIfIntersected(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, uint32_t vCodePoint,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	void ApplySelection(ProjectFile *vProjectFile,
		SelectionContainerEnum vSelectionContainerEnum);

private: // selection by line
	void SelectByLine(ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	bool DrawGlyphSelectionByLine(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, uint32_t vCodePoint,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // selection by zone
	void SelectByZone(ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	bool DrawGlyphSelectionByZone(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, uint32_t vCodePoint,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // selection by range
	void SelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		ImFontGlyph vGlyph,
        uint32_t vFontGlyphIndex,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		ImFontGlyph vGlyph,
        uint32_t vFontGlyphIndex,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		ImFontGlyph vGlyph, 
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);

public: // singleton
	static SelectionHelper *Instance()
	{
		static SelectionHelper *_instance = new SelectionHelper();
		return _instance;
	}

protected:
	SelectionHelper(); // Prevent construction
	SelectionHelper(const SelectionHelper&) {}; // Prevent construction by copying
	SelectionHelper& operator =(const SelectionHelper&) { return *this; }; // Prevent assignment
	~SelectionHelper(); // Prevent unwanted destruction
};


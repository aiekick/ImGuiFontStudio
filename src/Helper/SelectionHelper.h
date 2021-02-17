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
#include <ctools/ConfigAbstract.h>
#include <Project/BaseGlyph.h>

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

struct ReRangeCodePointLimitStruct
{
	bool valid = true; // for imgui color
	CodePoint codePoint = 0;
};

struct ReRangeCodePointStruct
{
	ReRangeCodePointLimitStruct startCodePoint;
	ReRangeCodePointLimitStruct endCodePoint;
	const uint32_t MinCodePoint = 0;
	const uint32_t MaxCodePoint = 65535;
};

class FontInfos;
struct ImGuiWindow;
typedef std::pair<GlyphIndex, std::shared_ptr<FontInfos>> FontInfosGlyphIndex;
struct TemporarySelectionStruct
{
	// for avoid selection apply if seletion ended outside of start window
	ImGuiWindow *startSelWindow = 0;

	std::set<FontInfosGlyphIndex> tmpSel;
	std::set<FontInfosGlyphIndex> tmpUnSel;

	bool isSelected(GlyphIndex c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosGlyphIndex(c, f);
		return (tmpSel.find(p) != tmpSel.end()); // found
	}
	
	bool isUnSelected(GlyphIndex c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosGlyphIndex(c, f);
		return (tmpUnSel.find(p) != tmpUnSel.end()); // found
	}
	void Select(GlyphIndex c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosGlyphIndex(c, f);
		tmpSel.emplace(p);
		tmpUnSel.erase(p);
	}
	void UnSelect(GlyphIndex c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosGlyphIndex(c, f);
		tmpSel.erase(p);
		tmpUnSel.emplace(p);
	}
	void Clear(GlyphIndex c, std::shared_ptr<FontInfos> f)
	{
		auto p = FontInfosGlyphIndex(c, f);
		tmpSel.erase(p);
		tmpUnSel.erase(p);
	}
	void Clear()
	{
		tmpSel.clear();
		tmpUnSel.clear();
	}
};

struct BaseGlyph;
class ProjectFile;
class SelectionHelper : public conf::ConfigAbstract
{
private: // Vars
	GlyphSelectionTypeFlags m_GlyphSelectionTypeFlags =	GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_ZONE;
	GlyphSelectionModeFlags m_GlyphSelectionModeFlags =	GlyphSelectionModeFlags::GLYPH_SELECTION_MODE_ADD;
	//start x,y, end z,w // if not active x,y,z,w == 0.0
	ct::fvec4 m_Line;
	// pos x,y, radius z, default radius // if not active x,y == 0,0
	ct::fvec4 m_Zone = ct::fvec4(0.0f, 0.0f, 0.5f, 0.5f);
	// selection for operations in final pane
	std::set<FontInfosGlyphIndex> m_SelectionForOperation;
	// first glyph state when clicked
	// if not selected will apply unseletion
	// if selected will apply selection
	int m_GlyphSelectedStateFirstClick = -1;

private: // structs / classes
	ReRangeCodePointStruct m_ReRangeCodePoint; // re range : change range of selection
	TemporarySelectionStruct m_TmpSelectionSrc; // for selection of glyphs from sources
	TemporarySelectionStruct m_TmpSelectionDst; // for operations on final seletected glyphs // like re range by glyph group

public:
	typedef std::pair<GlyphIndex, std::string> FontInfosGlyphIndex_ToLoad;
	std::set<FontInfosGlyphIndex_ToLoad> m_SelectionForOperation_ToLoad;

private:
	TemporarySelectionStruct* getSelStruct(SelectionContainerEnum vSelectionContainerEnum);
	bool IsGlyphSelected(
		std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum,
		GlyphIndex vGlyphIndex);
	void StartSelection(SelectionContainerEnum vSelectionContainerEnum);
	bool CanWeApplySelection(SelectionContainerEnum vSelectionContainerEnum);
	
private:
	static void DrawRect(ImVec2 vPos, ImVec2 vSize);
	static void DrawCircle(ImVec2 vPos, float vRadius);
	static void DrawLine(ImVec2 vStart, ImVec2 vEnd);

public:
	void DrawMenu(ProjectFile *vProjectFile);
	void DrawSelectionMenu(ProjectFile *vProjectFile, SelectionContainerEnum vSelectionContainerEnum);
	void Clear();
	void Load(ProjectFile* vProjectFile);

public:
	std::set<FontInfosGlyphIndex>* GetSelection();
public:
	void SelectWithToolOrApply(
		ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectWithToolOrApplyOnGlyph(
		ProjectFile *vProjectFile, 
		std::shared_ptr<FontInfos> vFontInfos, 
		BaseGlyph* vGlyph,
		GlyphIndex vGlyphIndex,
		bool vGlypSelected,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	bool IsGlyphIntersectedAndSelected(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCellSize, 
		GlyphIndex vGlyphIndex,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	bool IsSelectionMode(GlyphSelectionModeFlags vGlyphSelectionModeFlags);
	bool IsSelectionType(GlyphSelectionTypeFlags vGlyphSelectionTypeFlags);
	void AnalyseSourceSelection(ProjectFile *vProjectFile);

private:
	void SelectAllGlyphs(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, BaseGlyph* vGlyph, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, FontInfosGlyphIndex vFontInfosGlyphIndex, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void SelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, GlyphIndex vGlyphIndex, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	
	void UnSelectAllGlyphs(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, BaseGlyph* vGlyph, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, FontInfosGlyphIndex vFontInfosGlyphIndex, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyph(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos, GlyphIndex vGlyphIndex, bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);

private:
	void RemoveSelectionFromFinal(ProjectFile *vProjectFile);
	void ReRange_CodePoints_Offset_After_Start(ProjectFile *vProjectFile, GlyphIndex vOffsetGlyphIndex);
	void ReRange_CodePoints_Offset_Before_End(ProjectFile *vProjectFile, GlyphIndex vOffsetGlyphIndex);
	
private: // selection for view modes
	void PrepareSelection(ProjectFile *vProjectFile,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // ReRange
	void FinalizeSelectionForOperations();

private: // selections mode common
	void GlyphSelectionIfIntersected(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, GlyphIndex vGlyphIndex,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	void ApplySelection(ProjectFile *vProjectFile,
		SelectionContainerEnum vSelectionContainerEnum);

private: // selection by line
	void SelectByLine(ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	bool DrawGlyphSelectionByLine(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, GlyphIndex vGlyphIndex,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // selection by zone
	void SelectByZone(ProjectFile *vProjectFile, 
		SelectionContainerEnum vSelectionContainerEnum);
	bool DrawGlyphSelectionByZone(
		std::shared_ptr<FontInfos> vFontInfos,
		ImVec2 vCaseSize, GlyphIndex vGlyphIndex,
		bool *vSelected,
		SelectionContainerEnum vSelectionContainerEnum);
	
private: // selection by range
	void SelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		BaseGlyph* vGlyph,
		CodePoint vCodePoint,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		BaseGlyph* vGlyph,
		CodePoint vCodePoint,
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);
	void UnSelectGlyphByRangeFromStartCodePoint(
		ProjectFile *vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, 
		BaseGlyph* vGlyph, 
		bool vUpdateMaps,
		SelectionContainerEnum vSelectionContainerEnum);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

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


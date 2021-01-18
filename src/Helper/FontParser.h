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

#include "MemoryStream.h"
#include <imgui/imgui.h>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility> // std::pair

 //////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////

namespace FontAnalyser
{
	class TableDisplay
	{
	private:
		std::vector<std::vector<std::string>> array;
		ImGuiListClipper m_Clipper;

	public:
		void AddItem(std::string vItem, std::string vSize, std::string vInfos);
		void DisplayTable(const char* vTableLabel, size_t vMaxCount = 0);
	};

	//////////////////////////////////////

	class HeaderStruct : public TableDisplay
	{
	public:
		std::string scalerType;
		uint16_t numTables = 0;
		uint16_t searchRange = 0;
		uint16_t entrySelector = 0;
		uint16_t rangeShift = 0;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem);
	};

	//////////////////////////////////////

	class TableStruct : public TableDisplay
	{
	public:
		uint8_t tag[5] = {};
		uint32_t checkSum = 0;
		uint32_t offset = 0;
		uint32_t length = 0;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem);
	};
	
	//////////////////////////////////////

	class maxpTableStruct : public TableDisplay
	{
	public:
		MemoryStream::Fixed version;
		uint16_t numGlyphs = 0;
		uint16_t maxPoints = 0;
		uint16_t maxContours = 0;
		uint16_t maxComponentPoints = 0;
		uint16_t maxComponentContours = 0;
		uint16_t maxZones = 0;
		uint16_t maxTwilightPoints = 0;
		uint16_t maxStorage = 0;
		uint16_t maxFunctionDefs = 0;
		uint16_t maxInstructionDefs = 0;
		uint16_t maxStackElements = 0;
		uint16_t maxSizeOfInstructions = 0;
		uint16_t maxComponentElements = 0;
		uint16_t maxComponentDepth = 0;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class postTableF2Struct : public TableDisplay
	{
	public:
		uint16_t numberOfGlyphs = 0;
		std::vector<uint16_t> glyphNameIndex;
		std::vector<std::string> names;

	public:
		bool filled = false;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	class postTableStruct : public TableDisplay
	{
	public:
		MemoryStream::Fixed format;
		MemoryStream::Fixed italicAngle;
		MemoryStream::FWord underlinePosition = 0;
		MemoryStream::FWord underlineThickness = 0;
		uint32_t isFixedPitch = 0;
		uint32_t minMemType42 = 0;
		uint32_t maxMemType42 = 0;
		uint32_t minMemType1 = 0;
		uint32_t maxMemType1 = 0;
		postTableF2Struct tableF2;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class headTableStruct : public TableDisplay
	{
	public:
		MemoryStream::Fixed version;
		MemoryStream::Fixed fontRevision;
		uint32_t checkSumAdjustment;
		uint32_t magicNumber;
		uint16_t flags; // bitset
		uint16_t unitsPerEm;
		MemoryStream::longDateTime created;
		MemoryStream::longDateTime modified;
		MemoryStream::FWord xMin;
		MemoryStream::FWord yMin;
		MemoryStream::FWord xMax;
		MemoryStream::FWord yMax;
		uint16_t macStyle; // bitset
		uint16_t lowestRecPPEM;
		int16_t fontDirectionHint;
		int16_t indexToLocFormat;
		int16_t glyphDataFormat;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class cmapSubTableF4Struct : public TableDisplay
	{
	public:
		uint16_t format = 0;
		uint16_t length = 0;
		uint16_t language = 0;
		uint16_t segCountX2 = 0;
		uint16_t searchRange = 0;
		uint16_t entrySelector = 0;
		uint16_t rangeShift = 0;
		std::vector<uint16_t> endCode;
		uint16_t reservedPad = 0;
		std::vector<uint16_t> startCode;
		std::vector<int16_t> idDelta;
		std::vector<uint16_t> idRangeOffset;
		std::vector<uint16_t> glyphIdArray;

	public:
		bool filled = false;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	class cmapSubTableF0Struct : public TableDisplay
	{
	public:
		uint16_t format = 0;
		uint16_t length = 0;
		uint16_t language = 0;
		uint8_t glyphIndexArray[256]; // max256

	public:
		bool filled = false;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	class cmapEncodingRecordStruct : public TableDisplay
	{
	public:
		uint16_t platformID = 0;
		uint16_t encodingID = 0;
		uint32_t offset = 0;
		cmapSubTableF0Struct subTableF0;
		cmapSubTableF4Struct subTableF4;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	class cmapTableStruct : public TableDisplay
	{
	public:
		uint16_t version = 0;
		uint16_t numEncodingRecords = 0;
		std::vector<cmapEncodingRecordStruct> encodingRecords;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class locaTableStruct : public TableDisplay
	{
	public:
		headTableStruct *head = 0;
		maxpTableStruct *maxp = 0;

	public:
		std::vector<uint32_t> offsets;
		
	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class simpleGlyphTableStruct : public TableDisplay
	{
	public: 
		bool filled = false;

	public:
		std::vector<uint16_t> endPtsOfContours;
		uint16_t instructionLength;
		std::vector<uint8_t> instructions;
		std::vector<uint8_t> flags;
		std::vector<int16_t> xCoordinates;
		std::vector<int16_t> yCoordinates;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength, int16_t vCountContours);
	};

	class glyfStruct : public TableDisplay
	{
	public:
		int16_t numberOfContours = 0;
		MemoryStream::FWord xMin = 0;
		MemoryStream::FWord yMin = 0;
		MemoryStream::FWord xMax = 0;
		MemoryStream::FWord yMax = 0;
		simpleGlyphTableStruct simpleGlyph;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	class glyfTableStruct : public TableDisplay
	{
	public:
		locaTableStruct *loca = 0;

	public:
		std::vector<glyfStruct> glyfs;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class FontAnalyzedStruct
	{
	public:
		bool parsed = false;
		HeaderStruct header;
		std::map<std::string, TableStruct> tables;
		maxpTableStruct maxp;
		postTableStruct post;
		headTableStruct head;
		cmapTableStruct cmap;
		locaTableStruct loca;
		glyfTableStruct glyf;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream *vMem);
	};
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

class FontParser
{
private:
	FontAnalyser::FontAnalyzedStruct m_FontAnalyzed;

public:
	FontParser();
	~FontParser();

	void ParseFont(const std::string& vFilePathName);
	int draw(int vWidgetId);
};


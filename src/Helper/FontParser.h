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

#include <Generator/MemoryStream.h>
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

	class NameRecord : public TableDisplay
	{
	public:
		uint16_t platformID = 0;
		uint16_t encodingID = 0;
		uint16_t languageID = 0;
		uint16_t nameID = 0;
		uint16_t length = 0;
		uint16_t stringOffset = 0;

		std::string name;

		size_t GetSizeof() // cant use sizeof due to heritage of TableDisplay
		{
			return 12U; // (u16*6 = > 2*6 => 12)
		}

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class nameTableV0Struct : public TableDisplay
	{
	public:
		uint16_t count = 0;
		uint16_t storageOffset = 0;

		std::vector<NameRecord> nameRecords;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class LangTagRecordStruct : public TableDisplay
	{
	public:

		

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class nameTableV1Struct : public TableDisplay
	{
	public:
		uint16_t count = 0;
		uint16_t storageOffset = 0;
		std::vector<NameRecord> nameRecords;
		uint16_t langTagCount = 0;
		std::vector<LangTagRecordStruct> langTagRecords;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class nameTableStruct : public TableDisplay
	{
	public:
		uint16_t version = 0;
		
		nameTableV0Struct nameTableV0;
		nameTableV1Struct nameTableV1;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
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

	class baseGlyphRecordStruct : public TableDisplay
	{
	public:
		uint16_t glyphID;
		uint16_t firstLayerIndex;
		uint16_t numLayers;

		size_t GetSizeof() // cant use sizeof due to heritage of TableDisplay
		{
			return 6U; // (u16+u16+u16 = > 2+2+2)
		}

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class layerRecordStruct : public TableDisplay
	{
	public:
		uint16_t glyphID;
		uint16_t paletteIndex;

		size_t GetSizeof() // cant use sizeof due to heritage of TableDisplay
		{
			return 4U; // (u16+u16 = > 2+2)
		}

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class colrTableStruct : public TableDisplay
	{
	public:
		uint16_t version;
		uint16_t numBaseGlyphRecords;
		uint32_t baseGlyphRecordsOffset;
		uint32_t layerRecordsOffset;
		uint16_t numLayerRecords;

		size_t GetSizeof() // cant use sizeof due to heritage of TableDisplay
		{
			return 14U; // (u16+u16+u32+u32+u16 = > 2+2+4+4+2)
		}

	public:
		std::vector<baseGlyphRecordStruct> baseGlyphRecords;
		std::vector<layerRecordStruct> layerRecords;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	class colorRecordStruct : public TableDisplay
	{
	public:
		uint8_t blue;
		uint8_t green;
		uint8_t red;
		uint8_t alpha;

		size_t GetSizeof() // cant use sizeof due to heritage of TableDisplay
		{
			return 8U; // (u16+u16+u16+u16 = > 2+2+2+2)
		}

	private:
		ImVec4 color;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class paletteStruct : public TableDisplay
	{
	public:
		std::vector<colorRecordStruct> colorRecords;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};
	
	class cpalTableV0Struct : public TableDisplay
	{
	public:
		uint16_t numPaletteEntries;
		uint16_t numPalettes;
		uint16_t numColorRecords; // sum on each palette records count
		uint32_t colorRecordsArrayOffset;
	
	public:
		std::vector<uint16_t> colorRecordIndices; // numPalettes
		std::vector<paletteStruct> palettes;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class cpalTableV1Struct : public TableDisplay
	{
	public:
		uint16_t numPaletteEntries;
		uint16_t numPalettes;
		uint16_t numColorRecords;
		uint32_t colorRecordsArrayOffset;
		uint32_t paletteTypesArrayOffset;
		uint32_t paletteLabelsArrayOffset;
		uint32_t paletteEntryLabelsArrayOffset;

	public:
		std::vector<uint16_t> colorRecordIndices; // numPalettes

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	class cpalTableStruct : public TableDisplay
	{
	public:
		uint16_t version; // 0 => CPAL 0 / 1 => CPAL 1
		cpalTableV0Struct tableV0Struct;
		cpalTableV1Struct tableV1Struct;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength);
	};

	//////////////////////////////////////

	typedef int CompositeArgumentFlags;
	enum CompositeArgumentFlags_
	{
		ARG_1_AND_2_ARE_WORDS = (1 << 0),
		ARGS_ARE_XY_VALUES = (1 << 1),
		ROUND_XY_TO_GRID = (1 << 2),
		WE_HAVE_A_SCALE = (1 << 3),
		OBSOLETE = (1 << 4),
		MORE_COMPONENTS = (1 << 5),
		WE_HAVE_AN_X_AND_Y_SCALE = (1 << 6),
		WE_HAVE_A_TWO_BY_TWO = (1 << 7),
		WE_HAVE_INSTRUCTIONS = (1 << 8),
		USE_MY_METRICS = (1 << 9),
		OVERLAP_COMPOUND = (1 << 10),
		SCALED_COMPONENT_OFFSET = (1 << 11),
		UNSCALED_COMPONENT_OFFSET = (1 << 12)
	};

	class compositeGlyphTableStruct : public TableDisplay
	{
	public:
		bool filled = false;

	public:
		uint16_t flags;
		uint16_t glyphIndex;

		uint16_t argument1_16; 
		uint16_t argument2_16;

		uint8_t argument1_8;
		uint8_t argument2_8;

		MemoryStream::F2DOT14 scale;
		MemoryStream::F2DOT14 xscale;
		MemoryStream::F2DOT14 yscale;
		MemoryStream::F2DOT14 scale01;
		MemoryStream::F2DOT14 scale10;

	public:
		int draw(int vWidgetId);
		void parse(MemoryStream* vMem, size_t vOffset, size_t vLength, int16_t vCountContours);
	}; 
	
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
		compositeGlyphTableStruct compositeGlyph;

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
		nameTableStruct name;
		postTableStruct post;
		headTableStruct head;
		cmapTableStruct cmap;
		locaTableStruct loca;
		glyfTableStruct glyf;
		colrTableStruct colr;
		cpalTableStruct cpal;

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


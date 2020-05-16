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
 
#include "FontParser.h"
#include "MemoryStream.h"
#include "FileHelper.h"
#include "imgui/imgui.h"

using namespace FontAnalyser;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::HeaderStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Header"))
	{
		ImGui::Text("scalerType    (4 bytes) : %i", scalerType);
		ImGui::Text("numTables     (2 bytes) : %i", numTables);
		ImGui::Text("searchRange   (2 bytes) : %i", searchRange);
		ImGui::Text("entrySelector (2 bytes) : %i", entrySelector);
		ImGui::Text("rangeShift    (2 bytes) : %i", rangeShift);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::TableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode(&tag, "Table : %s", tag))
	{
		ImGui::Text("tag      (4 bytes) : %s", tag);
		ImGui::Text("checkSum (4 bytes) : %i", checkSum);
		ImGui::Text("offset   (4 bytes) : %i", offset);
		ImGui::Text("length   (4 bytes) : %i", length);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::maxpTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("maxp Table :"))
	{
		ImGui::Text("version               (4 bytes) : %i.%i", version.high, version.low);
		ImGui::Text("numGlyphs             (2 bytes) : %i", numGlyphs);
		ImGui::Text("maxPoints             (2 bytes) : %i", maxPoints);
		ImGui::Text("maxContours           (2 bytes) : %i", maxContours);
		ImGui::Text("maxComponentPoints    (2 bytes) : %i", maxComponentPoints);
		ImGui::Text("maxComponentContours  (2 bytes) : %i", maxComponentContours);
		ImGui::Text("maxZones              (2 bytes) : %i", maxZones);
		ImGui::Text("maxTwilightPoints     (2 bytes) : %i", maxTwilightPoints);
		ImGui::Text("maxStorage            (2 bytes) : %i", maxStorage);
		ImGui::Text("maxFunctionDefs       (2 bytes) : %i", maxFunctionDefs);
		ImGui::Text("maxInstructionDefs    (2 bytes) : %i", maxInstructionDefs);
		ImGui::Text("maxStackElements      (2 bytes) : %i", maxStackElements);
		ImGui::Text("maxSizeOfInstructions (2 bytes) : %i", maxSizeOfInstructions);
		ImGui::Text("maxComponentElements  (2 bytes) : %i", maxComponentElements);
		ImGui::Text("maxComponentDepth     (2 bytes) : %i", maxComponentDepth);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::cmapSubTableF0Struct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);

		if (ImGui::TreeNode("cmap Sub Table F0 :"))
		{
			ImGui::Text("format             (2 bytes) : %i", format);
			ImGui::Text("length             (2 bytes) : %i", length);
			ImGui::Text("language           (2 bytes) : %i", language);

			for (auto & it : glyphIndexArray)
			{
				ImGui::Text("glyph index :      (1 bytes) : %i", it);
			}
			
			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	
	return vWidgetId;
}

int FontAnalyser::cmapSubTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("cmap SubTable"))
	{
		ImGui::Text("platformID         (2 bytes) : %i", platformID);
		ImGui::Text("platformSpecificID (2 bytes) : %i", platformSpecificID);
		ImGui::Text("offset             (4 bytes) : %i", offset);
		
		vWidgetId = subTableF0.draw(vWidgetId);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::cmapTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("cmap Table (bad parse for the moment) :"))
	{
		ImGui::Text("version            (2 bytes) : %i", version);
		ImGui::Text("numberSubtables    (2 bytes) : %i", numberSubtables);

		for (auto & it : subTables)
		{
			vWidgetId = it.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::headTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("head Table :"))
	{
		ImGui::Text("version            (2 bytes) : %i", version);
		ImGui::Text("fontRevision       (2 bytes) : %i", fontRevision);
		ImGui::Text("checkSumAdjustment (4 bytes) : %i", checkSumAdjustment);
		ImGui::Text("magicNumber		(4 bytes) : %i", magicNumber);
		ImGui::Text("flags              (2 bytes) : %i", flags); // bitset
		ImGui::Text("unitsPerEm         (2 bytes) : %i", unitsPerEm);
		ImGui::Text("created            (8 bytes) : %i", created);
		ImGui::Text("modified           (8 bytes) : %i", modified);
		ImGui::Text("xMin               (2 bytes) : %i", xMin);
		ImGui::Text("yMin               (2 bytes) : %i", yMin);
		ImGui::Text("xMax               (2 bytes) : %i", xMax);
		ImGui::Text("yMax               (2 bytes) : %i", yMax);
		ImGui::Text("macStyle           (2 bytes) : %i", macStyle); // bitset
		ImGui::Text("lowestRecPPEM      (2 bytes) : %i", lowestRecPPEM);
		ImGui::Text("fontDirectionHint  (2 bytes) : %i", fontDirectionHint);
		ImGui::Text("indexToLocFormat   (2 bytes) : %i", indexToLocFormat);
		ImGui::Text("glyphDataFormat    (2 bytes) : %i", glyphDataFormat);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::postTableF2Struct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);

		if (ImGui::TreeNode("post Table Format 2 :"))
		{
			ImGui::Text("numberOfGlyphs  (4 bytes) : %i", numberOfGlyphs);

			if (ImGui::TreeNode("glyphNameIndexs (2 bytes array of numberOfGlyphs)"))
			{
				for (auto & it : glyphNameIndex)
				{
					ImGui::Text("glyph id        (2 bytes) : %i", it);
				}

				ImGui::TreePop();
			}

			//uint8_t space;

			if (ImGui::TreeNode("names :"))
			{
				for (auto & it : names)
				{
					ImGui::Text("%s", it.c_str());
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	return vWidgetId;
}

int FontAnalyser::postTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("post Table :"))
	{
		ImGui::Text("format             (4 bytes) : %i.%i", format.high, format.low);
		ImGui::Text("italicAngle        (4 bytes) : %i.%i", italicAngle.high, italicAngle.low);
		ImGui::Text("underlinePosition  (2 bytes) : %i", underlinePosition);
		ImGui::Text("underlineThickness (2 bytes) : %i", underlineThickness);
		ImGui::Text("isFixedPitch       (4 bytes) : %i", isFixedPitch);
		ImGui::Text("minMemType42       (4 bytes) : %i", minMemType42);
		ImGui::Text("maxMemType42       (4 bytes) : %i", maxMemType42);
		ImGui::Text("minMemType1        (4 bytes) : %i", minMemType1);
		ImGui::Text("maxMemType1        (4 bytes) : %i", maxMemType1);

		if (tableF2.filled)
		{
			vWidgetId = tableF2.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

int FontAnalyser::FontAnalyzedStruct::draw(int vWidgetId)
{
	if (parsed)
	{
		vWidgetId = header.draw(vWidgetId);

		ImGui::Separator();

		for (auto & it : tables)
		{
			vWidgetId = it.second.draw(vWidgetId);
		}

		ImGui::Separator();

		vWidgetId = maxp.draw(vWidgetId);
		vWidgetId = post.draw(vWidgetId);
		vWidgetId = head.draw(vWidgetId);
		vWidgetId = cmap.draw(vWidgetId);
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const char *FontParserStandardMacNames[258] = { ".notdef", ".null", "nonmarkingreturn", "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle", "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", "at", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "grave", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute", "agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", "ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", "ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls", "registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash", "infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", "product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash", "questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft", "guillemotright", "ellipsis", "nonbreakingspace", "Agrave", "Atilde", "Otilde", "OE", "oe", "endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge", "ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl", "daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute", "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", "apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", "macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", "Lslash", "lslash", "Scaron", "scaron", "Zcaron", "zcaron", "brokenbar", "Eth", "eth", "Yacute", "yacute", "Thorn", "thorn", "minus", "multiply", "onesuperior", "twosuperior", "threesuperior", "onehalf", "onequarter", "threequarters", "franc", "Gbreve", "gbreve", "Idotaccent", "Scedilla", "scedilla", "Cacute", "cacute", "Ccaron", "ccaron", "dcroat" };

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FontParser::FontParser()
{

}

FontParser::~FontParser()
{

}

void FontParser::ParseFont(const std::string& vFilePathName)
{
	auto arrBytes = FileHelper::Instance()->LoadFileToBytes(vFilePathName);
	if (!arrBytes.empty())
	{
		MemoryStream mem;
		mem.Set(arrBytes.data(), arrBytes.size());

		m_FontAnalyzed.header.scalerType = mem.ReadULong();
		m_FontAnalyzed.header.numTables = mem.ReadUShort();
		m_FontAnalyzed.header.searchRange = mem.ReadUShort();
		m_FontAnalyzed.header.entrySelector = mem.ReadUShort();
		m_FontAnalyzed.header.rangeShift = mem.ReadUShort();

		for (int i = 0; i < m_FontAnalyzed.header.numTables; i++)
		{
			TableStruct tbl;
			uint32_t tag = mem.ReadULong();
			tbl.tag[0] = (uint8_t)((tag >> 24) & 0xff);
			tbl.tag[1] = (uint8_t)((tag >> 16) & 0xff);
			tbl.tag[2] = (uint8_t)((tag >> 8) & 0xff);
			tbl.tag[3] = (uint8_t)(tag & 0xff);
			tbl.tag[4] = '\0';

			tbl.checkSum = mem.ReadULong();
			tbl.offset = mem.ReadULong();
			tbl.length = mem.ReadULong();
			m_FontAnalyzed.tables[std::string((char*)tbl.tag)] = tbl;
		}

		if (m_FontAnalyzed.tables.find("maxp") != m_FontAnalyzed.tables.end())
		{
			mem.SetPos(m_FontAnalyzed.tables["maxp"].offset);
		
			uint32_t version = mem.ReadFixed();
			m_FontAnalyzed.maxp.version.high = (uint16_t)((version >> 16) & 0xff);
			m_FontAnalyzed.maxp.version.low = (uint16_t)(version & 0xff);
			m_FontAnalyzed.maxp.numGlyphs = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxPoints = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxContours = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxComponentPoints = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxComponentContours = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxZones = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxTwilightPoints = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxStorage = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxFunctionDefs = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxInstructionDefs = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxStackElements = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxSizeOfInstructions = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxComponentElements = mem.ReadUShort();
			m_FontAnalyzed.maxp.maxComponentDepth = mem.ReadUShort();
		}

		if (m_FontAnalyzed.tables.find("head") != m_FontAnalyzed.tables.end())
		{
			mem.SetPos(m_FontAnalyzed.tables["head"].offset);

			m_FontAnalyzed.head.version = mem.ReadShort();
			m_FontAnalyzed.head.fontRevision = mem.ReadShort();
			m_FontAnalyzed.head.checkSumAdjustment = mem.ReadULong();
			m_FontAnalyzed.head.magicNumber = mem.ReadULong();
			m_FontAnalyzed.head.flags = mem.ReadUShort(); // bitset
			m_FontAnalyzed.head.unitsPerEm = mem.ReadUShort();
			m_FontAnalyzed.head.created = mem.ReadDateTimeAsLong();
			m_FontAnalyzed.head.modified = mem.ReadDateTimeAsLong();
			m_FontAnalyzed.head.xMin = mem.ReadShort();
			m_FontAnalyzed.head.yMin = mem.ReadShort();
			m_FontAnalyzed.head.xMax = mem.ReadShort();
			m_FontAnalyzed.head.yMax = mem.ReadShort();
			m_FontAnalyzed.head.macStyle = mem.ReadUShort(); // bitset
			m_FontAnalyzed.head.lowestRecPPEM = mem.ReadUShort();
			m_FontAnalyzed.head.fontDirectionHint = mem.ReadShort();
			m_FontAnalyzed.head.indexToLocFormat = mem.ReadShort();
			m_FontAnalyzed.head.glyphDataFormat = mem.ReadShort();
		}

		if (m_FontAnalyzed.tables.find("post") != m_FontAnalyzed.tables.end())
		{
			size_t startPos = m_FontAnalyzed.tables["post"].offset;
			size_t endPos = startPos + m_FontAnalyzed.tables["post"].length;
			mem.SetPos(startPos);

			uint32_t format = mem.ReadFixed();
			m_FontAnalyzed.post.format.high = (uint16_t)((format >> 16) & 0xff);
			m_FontAnalyzed.post.format.low = (uint16_t)(format & 0xff);
			uint32_t italicAngle = mem.ReadFixed();
			m_FontAnalyzed.post.italicAngle.high = (uint16_t)((italicAngle >> 16) & 0xff);
			m_FontAnalyzed.post.italicAngle.low = (uint16_t)(italicAngle & 0xff);
			m_FontAnalyzed.post.underlinePosition = mem.ReadUShort();
			m_FontAnalyzed.post.underlineThickness = mem.ReadUShort();
			m_FontAnalyzed.post.isFixedPitch = mem.ReadULong();
			m_FontAnalyzed.post.minMemType42 = mem.ReadULong();
			m_FontAnalyzed.post.maxMemType42 = mem.ReadULong();
			m_FontAnalyzed.post.minMemType1 = mem.ReadULong();
			m_FontAnalyzed.post.maxMemType1 = mem.ReadULong();

			m_FontAnalyzed.post.tableF2.filled = false;

			if (m_FontAnalyzed.post.format.high == 2)
			{
				m_FontAnalyzed.post.tableF2.filled = true;

				std::vector<std::string> pendingNames;

				m_FontAnalyzed.post.tableF2.numberOfGlyphs = mem.ReadUShort();
				for (int i = 0; i < m_FontAnalyzed.post.tableF2.numberOfGlyphs; i++)
				{
					m_FontAnalyzed.post.tableF2.glyphNameIndex.push_back(mem.ReadUShort());
				}

				while (mem.GetPos() < endPos)
				{
					uint8_t len = mem.ReadByte();
					std::string str = mem.ReadString(len);
					pendingNames.push_back(str);
				}

				for (int i = 0; i < m_FontAnalyzed.post.tableF2.numberOfGlyphs; i++)
				{
					uint16_t mapIdx = m_FontAnalyzed.post.tableF2.glyphNameIndex[i];
					if (mapIdx >= 258)
					{
						uint16_t idx = mapIdx - 258;
						if (idx < pendingNames.size())
							m_FontAnalyzed.post.tableF2.names.push_back(pendingNames[idx]);
					}
					else
					{
						m_FontAnalyzed.post.tableF2.names.emplace_back(FontParserStandardMacNames[mapIdx]);
					}
				}
			}
		}

		if (m_FontAnalyzed.tables.find("cmap") != m_FontAnalyzed.tables.end())
		{
			size_t startPos = m_FontAnalyzed.tables["cmap"].offset;
			mem.SetPos(startPos);

			m_FontAnalyzed.cmap.version = mem.ReadUShort();
			m_FontAnalyzed.cmap.numberSubtables = mem.ReadUShort();

			for (int i = 0; i < m_FontAnalyzed.cmap.numberSubtables; i++)
			{
				cmapSubTableStruct tbl;
				tbl.platformID = mem.ReadUShort();
				tbl.platformSpecificID = mem.ReadUShort();
				tbl.offset = mem.ReadULong();

				//mem.SetPos(tbl.offset);
				
				tbl.subTableF0.filled = false;

				uint16_t format = mem.ReadUShort();

				if (format == 0)
				{
					tbl.subTableF0.filled = true;
					tbl.subTableF0.format = format;
					tbl.subTableF0.length = mem.ReadUShort();
					tbl.subTableF0.language = mem.ReadUShort();
					
					for (int j = 0; j < 256; j++)
					{
						tbl.subTableF0.glyphIndexArray.push_back(mem.ReadByte());
					}
				}
				else 
				{
					ImGui::Text("Not added for the moment");
				}

				m_FontAnalyzed.cmap.subTables.push_back(tbl);
			}

			m_FontAnalyzed.head.checkSumAdjustment = mem.ReadULong();
			m_FontAnalyzed.head.magicNumber = mem.ReadULong();
			m_FontAnalyzed.head.flags = mem.ReadUShort(); // bitset
			m_FontAnalyzed.head.unitsPerEm = mem.ReadUShort();
			m_FontAnalyzed.head.created = mem.ReadDateTimeAsLong();
			m_FontAnalyzed.head.modified = mem.ReadDateTimeAsLong();
			m_FontAnalyzed.head.xMin = mem.ReadShort();
			m_FontAnalyzed.head.yMin = mem.ReadShort();
			m_FontAnalyzed.head.xMax = mem.ReadShort();
			m_FontAnalyzed.head.yMax = mem.ReadShort();
			m_FontAnalyzed.head.macStyle = mem.ReadUShort(); // bitset
			m_FontAnalyzed.head.lowestRecPPEM = mem.ReadUShort();
			m_FontAnalyzed.head.fontDirectionHint = mem.ReadShort();
			m_FontAnalyzed.head.indexToLocFormat = mem.ReadShort();
			m_FontAnalyzed.head.glyphDataFormat = mem.ReadShort();
		}

		/////////////////////////////
		m_FontAnalyzed.parsed = true;
	}
}

int FontParser::draw(int vWidgetId)
{
	return m_FontAnalyzed.draw(vWidgetId);
}
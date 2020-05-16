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
#include "FileHelper.h"
#include "imgui/imgui.h"

using namespace FontAnalyser;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::HeaderStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Header"))
	{
		ImGui::Text("scalerType    (4 bytes) : %s", (scalerType[0] == 0?(scalerType[1] == 1?"TRUE":""): scalerType.c_str()));
		ImGui::Text("numTables     (2 bytes) : %i", numTables);
		ImGui::Text("searchRange   (2 bytes) : %i", searchRange);
		ImGui::Text("entrySelector (2 bytes) : %i", entrySelector);
		ImGui::Text("rangeShift    (2 bytes) : %i", rangeShift);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::HeaderStruct::parse(MemoryStream *vMem)
{
	if (vMem)
	{
		scalerType = vMem->ReadString(4);
		numTables = vMem->ReadUShort();
		searchRange = vMem->ReadUShort();
		entrySelector = vMem->ReadUShort();
		rangeShift = vMem->ReadUShort();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

void FontAnalyser::TableStruct::parse(MemoryStream *vMem)
{
	if (vMem)
	{
		uint32_t _tag = vMem->ReadULong();
		tag[0] = (uint8_t)((_tag >> 24) & 0xff);
		tag[1] = (uint8_t)((_tag >> 16) & 0xff);
		tag[2] = (uint8_t)((_tag >> 8) & 0xff);
		tag[3] = (uint8_t)(_tag & 0xff);
		tag[4] = '\0';

		checkSum = vMem->ReadULong();
		offset = vMem->ReadULong();
		length = vMem->ReadULong();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::locaTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("loca Table :"))
	{
		for (auto & it : shortVersion)
		{
			ImGui::Text("offsets      (2 bytes) : %hu", it);
		}

		for (auto & it : longVersion)
		{
			ImGui::Text("offsets      (4 bytes) : %u", it);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::locaTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem && head && maxp)
	{
		vMem->SetPos(vOffset);

		if (head->indexToLocFormat == 0) // short format
		{
			for (int i = 0; i < maxp->numGlyphs; i++)
			{
				shortVersion.push_back(vMem->ReadUShort());
			}
		}
		else if (head->indexToLocFormat == 1) // long format
		{
			for (int i = 0; i < maxp->numGlyphs; i++)
			{
				longVersion.push_back(vMem->ReadULong());
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::maxpTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("maxp Table :"))
	{
		ImGui::Text("version               (4 bytes) : %hi.%hi", version.high, version.low);
		ImGui::Text("numGlyphs             (2 bytes) : %hu", numGlyphs);
		ImGui::Text("maxPoints             (2 bytes) : %hu", maxPoints);
		ImGui::Text("maxContours           (2 bytes) : %hu", maxContours);
		ImGui::Text("maxComponentPoints    (2 bytes) : %hu", maxComponentPoints);
		ImGui::Text("maxComponentContours  (2 bytes) : %hu", maxComponentContours);
		ImGui::Text("maxZones              (2 bytes) : %hu", maxZones);
		ImGui::Text("maxTwilightPoints     (2 bytes) : %hu", maxTwilightPoints);
		ImGui::Text("maxStorage            (2 bytes) : %hu", maxStorage);
		ImGui::Text("maxFunctionDefs       (2 bytes) : %hu", maxFunctionDefs);
		ImGui::Text("maxInstructionDefs    (2 bytes) : %hu", maxInstructionDefs);
		ImGui::Text("maxStackElements      (2 bytes) : %hu", maxStackElements);
		ImGui::Text("maxSizeOfInstructions (2 bytes) : %hu", maxSizeOfInstructions);
		ImGui::Text("maxComponentElements  (2 bytes) : %hu", maxComponentElements);
		ImGui::Text("maxComponentDepth     (2 bytes) : %hu", maxComponentDepth);

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::maxpTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = vMem->ReadFixed();
		numGlyphs = vMem->ReadUShort();
		maxPoints = vMem->ReadUShort();
		maxContours = vMem->ReadUShort();
		maxComponentPoints = vMem->ReadUShort();
		maxComponentContours = vMem->ReadUShort();
		maxZones = vMem->ReadUShort();
		maxTwilightPoints = vMem->ReadUShort();
		maxStorage = vMem->ReadUShort();
		maxFunctionDefs = vMem->ReadUShort();
		maxInstructionDefs = vMem->ReadUShort();
		maxStackElements = vMem->ReadUShort();
		maxSizeOfInstructions = vMem->ReadUShort();
		maxComponentElements = vMem->ReadUShort();
		maxComponentDepth = vMem->ReadUShort();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::cmapSubTableF0Struct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);

		if (ImGui::TreeNode("cmap Sub Table F0 :"))
		{
			ImGui::Text("format             (2 bytes) : %hu", format);
			ImGui::Text("length             (2 bytes) : %hu", length);
			ImGui::Text("language           (2 bytes) : %hu", language);

			for (auto & it : glyphIndexArray)
			{
				ImGui::Text("glyph index :      (1 bytes) : %hu", it);
			}
			
			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	
	return vWidgetId;
}

void FontAnalyser::cmapSubTableF0Struct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{

	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::cmapSubTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("cmap SubTable"))
	{
		ImGui::Text("platformID         (2 bytes) : %hu", platformID);
		ImGui::Text("platformSpecificID (2 bytes) : %hu", platformSpecificID);
		ImGui::Text("offset             (4 bytes) : %u", offset);
		
		vWidgetId = subTableF0.draw(vWidgetId);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::cmapSubTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		platformID = vMem->ReadUShort();
		platformSpecificID = vMem->ReadUShort();
		offset = vMem->ReadULong();

		uint16_t format = vMem->ReadUShort();

		if (format == 0)
		{
			subTableF0.filled = true;
			subTableF0.format = format;
			subTableF0.length = vMem->ReadUShort();
			subTableF0.language = vMem->ReadUShort();

			for (int j = 0; j < 256; j++)
			{
				subTableF0.glyphIndexArray.push_back(vMem->ReadByte());
			}
		}
		else
		{
			ImGui::Text("Not added for the moment");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::cmapTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("cmap Table (bad parse for the moment) :"))
	{
		ImGui::Text("version            (2 bytes) : %hu", version);
		ImGui::Text("numberSubtables    (2 bytes) : %hu", numberSubtables);

		for (auto & it : subTables)
		{
			vWidgetId = it.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::cmapTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = vMem->ReadUShort();
		numberSubtables = vMem->ReadUShort();

		for (int i = 0; i < numberSubtables; i++)
		{
			cmapSubTableStruct tbl;
			tbl.parse(vMem, 0, 0);
			subTables.push_back(tbl);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::headTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("head Table :"))
	{
		ImGui::Text("version            (4 bytes) : %hi.%hi", version.high, version.low);
		ImGui::Text("fontRevision       (4 bytes) : %hi.%hi", fontRevision.high, fontRevision.low);
		ImGui::Text("checkSumAdjustment (4 bytes) : %u", checkSumAdjustment);
		ImGui::Text("magicNumber		(4 bytes) : %X%s", magicNumber, (magicNumber==0x5F0F3CF5?" (Valid)":" (Not Valid)"));
		ImGui::Text("flags              (2 bytes) : %hu", flags); // bitset
		ImGui::Text("unitsPerEm         (2 bytes) : %hu", unitsPerEm);
		ImGui::Text("created            (8 bytes) : %lli", created);
		ImGui::Text("modified           (8 bytes) : %lli", modified);
		ImGui::Text("xMin               (2 bytes) : %hi", xMin);
		ImGui::Text("yMin               (2 bytes) : %hi", yMin);
		ImGui::Text("xMax               (2 bytes) : %hi", xMax);
		ImGui::Text("yMax               (2 bytes) : %hi", yMax);
		ImGui::Text("macStyle           (2 bytes) : %hu", macStyle); // bitset
		ImGui::Text("lowestRecPPEM      (2 bytes) : %hu", lowestRecPPEM);
		ImGui::Text("fontDirectionHint  (2 bytes) : %hi", fontDirectionHint);
		ImGui::Text("indexToLocFormat   (2 bytes) : %hi", indexToLocFormat);
		ImGui::Text("glyphDataFormat    (2 bytes) : %hi", glyphDataFormat);

		ImGui::TreePop();
	}

	ImGui::Separator();

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::headTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = vMem->ReadFixed();
		fontRevision = vMem->ReadFixed();
		checkSumAdjustment = vMem->ReadULong();
		magicNumber = vMem->ReadULong();
		flags = vMem->ReadUShort(); // bitset
		unitsPerEm = vMem->ReadUShort();
		created = vMem->ReadDateTime();
		modified = vMem->ReadDateTime();
		xMin = vMem->ReadFWord();
		yMin = vMem->ReadFWord();
		xMax = vMem->ReadFWord();
		yMax = vMem->ReadFWord();
		macStyle = vMem->ReadUShort(); // bitset
		lowestRecPPEM = vMem->ReadUShort();
		fontDirectionHint = vMem->ReadShort();
		indexToLocFormat = vMem->ReadShort();
		glyphDataFormat = vMem->ReadShort();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static const char *postTable_StandardMacNames[258] =
{
	".notdef", ".null", "nonmarkingreturn", "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand",
	"quotesingle", "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
	"three", "four", "five", "six", "seven", "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
	"at", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W",
	"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "grave", "a", "b", "c", "d", "e",
	"f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft",
	"bar", "braceright", "asciitilde", "Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis",
	"aacute", "agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", "ecircumflex",
	"edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", "ograve", "ocircumflex", "odieresis",
	"otilde", "uacute", "ugrave", "ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling", "section", "bullet",
	"paragraph", "germandbls", "registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash",
	"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", "product", "pi",
	"integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash", "questiondown", "exclamdown", "logicalnot",
	"radical", "florin", "approxequal", "Delta", "guillemotleft", "guillemotright", "ellipsis", "nonbreakingspace",
	"Agrave", "Atilde", "Otilde", "OE", "oe", "endash", "emdash", "quotedblleft", "quotedblright", "quoteleft",
	"quoteright", "divide", "lozenge", "ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft",
	"guilsinglright", "fi", "fl", "daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand",
	"Acircumflex", "Ecircumflex", "Aacute", "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave",
	"Oacute", "Ocircumflex", "apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde",
	"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", "Lslash", "lslash", "Scaron",
	"scaron", "Zcaron", "zcaron", "brokenbar", "Eth", "eth", "Yacute", "yacute", "Thorn", "thorn", "minus", "multiply",
	"onesuperior", "twosuperior", "threesuperior", "onehalf", "onequarter", "threequarters", "franc", "Gbreve", "gbreve",
	"Idotaccent", "Scedilla", "scedilla", "Cacute", "cacute", "Ccaron", "ccaron", "dcroat"
};

int FontAnalyser::postTableF2Struct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);
		if (ImGui::TreeNode("post Table Format 2 :"))
		{
			ImGui::Text("numberOfGlyphs  (2 bytes) : %hu", numberOfGlyphs);
			if (ImGui::TreeNode("glyphNameIndexs (2 bytes array of numberOfGlyphs)"))
			{
				for (auto & it : glyphNameIndex)
					ImGui::Text("glyph id        (2 bytes) : %hu", it);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("names :"))
			{
				for (auto & it : names)
					ImGui::Text("%s", it.c_str());
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	return vWidgetId;
}

void FontAnalyser::postTableF2Struct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		numberOfGlyphs = vMem->ReadUShort();
		if (numberOfGlyphs)
		{
			for (int i = 0; i < numberOfGlyphs; i++)
			{
				glyphNameIndex.push_back(vMem->ReadUShort());
			}

			size_t endPos = vOffset + vLength;

			std::vector<std::string> pendingNames;
			while (vMem->GetPos() < endPos)
			{
				uint8_t len = vMem->ReadByte();
				std::string str = vMem->ReadString(len);
				pendingNames.push_back(str);
			}

			for (int i = 0; i < numberOfGlyphs; i++)
			{
				uint16_t mapIdx = glyphNameIndex[i];
				if (mapIdx >= 258)
				{
					uint16_t idx = mapIdx - 258;
					if (idx < pendingNames.size())
						names.push_back(pendingNames[idx]);
				}
				else
				{
					names.emplace_back(postTable_StandardMacNames[mapIdx]);
				}
			}

			filled = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::postTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("post Table :"))
	{
		ImGui::Text("format             (4 bytes) : %hi.%hi", format.high, format.low);
		ImGui::Text("italicAngle        (4 bytes) : %hi.%hi", italicAngle.high, italicAngle.low);
		ImGui::Text("underlinePosition  (2 bytes) : %hi", underlinePosition);
		ImGui::Text("underlineThickness (2 bytes) : %hi", underlineThickness);
		ImGui::Text("isFixedPitch       (4 bytes) : %u", isFixedPitch);
		ImGui::Text("minMemType42       (4 bytes) : %u", minMemType42);
		ImGui::Text("maxMemType42       (4 bytes) : %u", maxMemType42);
		ImGui::Text("minMemType1        (4 bytes) : %u", minMemType1);
		ImGui::Text("maxMemType1        (4 bytes) : %u", maxMemType1);

		vWidgetId = tableF2.draw(vWidgetId);

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::postTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		format = vMem->ReadFixed();;
		italicAngle = vMem->ReadFixed();
		underlinePosition = vMem->ReadUShort();
		underlineThickness = vMem->ReadUShort();
		isFixedPitch = vMem->ReadULong();
		minMemType42 = vMem->ReadULong();
		maxMemType42 = vMem->ReadULong();
		minMemType1 = vMem->ReadULong();
		maxMemType1 = vMem->ReadULong();

		if (format.high == 2)
			tableF2.parse(vMem, vOffset, vLength);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
		for (auto & it : tables)
		{
			if (it.first == "head")	vWidgetId = head.draw(vWidgetId);
			else if (it.first == "maxp") vWidgetId = maxp.draw(vWidgetId);
			else if (it.first == "cmap") vWidgetId = cmap.draw(vWidgetId);
			else if (it.first == "loca") vWidgetId = loca.draw(vWidgetId);
			else if (it.first == "post") vWidgetId = post.draw(vWidgetId);
		}
	}

	return vWidgetId;
}


void FontAnalyser::FontAnalyzedStruct::parse(MemoryStream *vMem)
{
	if (vMem)
	{
#define IF_TABLE(_tag_) if (tables.find(_tag_) != tables.end())
#define PARSE_TABLE(_tag_, _class_) _class_.parse(vMem, tables[_tag_].offset, tables[_tag_].length)

		header.parse(vMem);

		for (int i = 0; i < header.numTables; i++)
		{
			TableStruct tbl;
			tbl.parse(vMem);
			tables[std::string((char*)tbl.tag)] = tbl;
		}

		IF_TABLE("head") PARSE_TABLE("head", head);
		IF_TABLE("maxp") PARSE_TABLE("maxp", maxp);
		IF_TABLE("cmap") PARSE_TABLE("cmap", cmap);
		IF_TABLE("post") PARSE_TABLE("post", post);
		IF_TABLE("loca") IF_TABLE("head") IF_TABLE("maxp")
		{
			loca.head = &head;
			loca.maxp = &maxp;
			PARSE_TABLE("loca", loca);
		}
		
#undef PARSE_TABLE
#undef IF_TABLE
	}

	/////////////////////////////
	parsed = true;
}

///////////////////////////////////////////////////////////////////////////////
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
		
		m_FontAnalyzed = {}; // re init
		m_FontAnalyzed.parse(&mem);
	}
}

int FontParser::draw(int vWidgetId)
{
	return m_FontAnalyzed.draw(vWidgetId);
}
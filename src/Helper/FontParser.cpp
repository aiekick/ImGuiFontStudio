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
#include <ctools/FileHelper.h>
#include <imgui/imgui.h>

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
		if (scalerType[0] == 1)  scalerType = "TrueType 1"; // TrueType 1
		if (scalerType[1] == 1)  scalerType = "OpenType 1"; // TrueType 1
		numTables = (uint16_t)vMem->ReadUShort();
		searchRange = (uint16_t)vMem->ReadUShort();
		entrySelector = (uint16_t)vMem->ReadUShort();
		rangeShift = (uint16_t)vMem->ReadUShort();
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
		uint32_t _tag = (uint32_t)vMem->ReadULong();
		tag[0] = (uint8_t)((_tag >> 24) & 0xff);
		tag[1] = (uint8_t)((_tag >> 16) & 0xff);
		tag[2] = (uint8_t)((_tag >> 8) & 0xff);
		tag[3] = (uint8_t)(_tag & 0xff);
		tag[4] = '\0';

		checkSum = (uint32_t)vMem->ReadULong();
		offset = (uint32_t)vMem->ReadULong();
		length = (uint32_t)vMem->ReadULong();
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
		if (head)
		{
			if (head->indexToLocFormat == 0) // short format
			{
				for (auto & it : offsets)
				{
					ImGui::Text("offsets      (2 bytes) : %hu", it);
				}
			}
			else if (head->indexToLocFormat == 1) // long format
			{
				for (auto & it : offsets)
				{
					ImGui::Text("offsets      (4 bytes) : %u", it);
				}
			}
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::locaTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t /*vLength*/)
{
	if (vMem && head && maxp)
	{
		vMem->SetPos(vOffset);

		if (head->indexToLocFormat == 0) // short format
		{
			for (int i = 0; i < maxp->numGlyphs; i++)
			{
				offsets.push_back(((uint32_t)vMem->ReadUShort()) * 2);
			}
		}
		else if (head->indexToLocFormat == 1) // long format
		{
			for (int i = 0; i < maxp->numGlyphs; i++)
			{
				offsets.push_back((uint32_t)vMem->ReadULong());
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

void FontAnalyser::maxpTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t /*vLength*/)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = vMem->ReadFixed();
		numGlyphs = (uint16_t)vMem->ReadUShort();
		maxPoints = (uint16_t)vMem->ReadUShort();
		maxContours = (uint16_t)vMem->ReadUShort();
		maxComponentPoints = (uint16_t)vMem->ReadUShort();
		maxComponentContours = (uint16_t)vMem->ReadUShort();
		maxZones = (uint16_t)vMem->ReadUShort();
		maxTwilightPoints = (uint16_t)vMem->ReadUShort();
		maxStorage = (uint16_t)vMem->ReadUShort();
		maxFunctionDefs = (uint16_t)vMem->ReadUShort();
		maxInstructionDefs = (uint16_t)vMem->ReadUShort();
		maxStackElements = (uint16_t)vMem->ReadUShort();
		maxSizeOfInstructions = (uint16_t)vMem->ReadUShort();
		maxComponentElements = (uint16_t)vMem->ReadUShort();
		maxComponentDepth = (uint16_t)vMem->ReadUShort();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::simpleGlyphTableStruct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);

		if (ImGui::TreeNode("Simple Glyph :"))
		{
			if (ImGui::TreeNode("endPtsOfContours :"))
			{
				for (auto & it : endPtsOfContours)
				{
					ImGui::Text("End pt            (1 byte) : %hu", it);
				}

				ImGui::TreePop();
			}
			ImGui::Text("instructionLength (2 bytes) : %hu", instructionLength);
			if (instructionLength)
			{
				if (ImGui::TreeNode("instructions :"))
				{
					for (auto & it : endPtsOfContours)
					{
						ImGui::Text("Instruction       (1 byte) : %hu", it);
					}

					ImGui::TreePop();
				}
			}
			if (ImGui::TreeNode("Flags :"))
			{
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
				for (auto & it : flags)
				{
					ImGui::Text("flag              (1 byte) :" BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(it));
				}
#undef BYTE_TO_BINARY
#undef BYTE_TO_BINARY_PATTERN
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("xCoordinates :"))
			{
				int shortX = (1 << 1);
				int idx = 0;
				for (auto & it : xCoordinates)
				{
					if (flags[idx] & shortX)
						ImGui::Text("xCoordinates      (1 byte) : %hi", it);
					else
						ImGui::Text("xCoordinates      (2 byte) : %hi", it);
					idx++;
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("yCoordinates :"))
			{
				int shortY = (1 << 2);
				int idx = 0;
				for (auto & it : yCoordinates)
				{
					if (flags[idx] & shortY)
						ImGui::Text("yCoordinates      (1 byte) : %hi", it);
					else
						ImGui::Text("yCoordinates      (2 byte) : %hi", it);
					idx++;
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	return vWidgetId;
}

void FontAnalyser::simpleGlyphTableStruct::parse(MemoryStream *vMem, size_t /*vOffset*/, size_t /*vLength*/, int16_t vCountContours)
{
	if (vMem)
	{
		if (vCountContours)
		{
			filled = true;
			
			for (int i = 0; i < vCountContours; i++)
			{
				endPtsOfContours.push_back((uint16_t)vMem->ReadShort());
			}

			instructionLength = (uint16_t)vMem->ReadUShort();
			
			for (int i = 0; i < instructionLength; i++)
			{
				instructions.push_back(vMem->ReadByte());
			}
			
			if (!endPtsOfContours.empty())
			{
				int countPoints = endPtsOfContours[endPtsOfContours.size() - 1];
				if (countPoints > 0)
				{
					uint32_t flag_repeat = 0;
					int flag = 0;
					for (int i = 0; i < countPoints; i++)
					{
						if (flag_repeat == 0)
						{
							flag = vMem->ReadByte();
							if ((flag & (1 << 3)) == (1 << 3))
							{
								flag_repeat = vMem->ReadByte();
							}
						}
						else
						{
							flag_repeat--;
						}
						flags.push_back((uint8_t)flag);
					}
					int shortX = (1 << 1);
					int shortY = (1 << 2);
					for (auto & it : flags)
					{
						if (it & shortX)
							xCoordinates.push_back((int16_t)vMem->ReadByte());
						else
							xCoordinates.push_back((int16_t)vMem->ReadShort());

						if (it & shortY)
							yCoordinates.push_back((int16_t)vMem->ReadByte());
						else
							yCoordinates.push_back((int16_t)vMem->ReadShort());
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::glyfStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("glyf :"))
	{
		ImGui::Text("numberOfContours (2 bytes) : %hu", numberOfContours);
		ImGui::Text("xMin             (2 bytes) : %hi", xMin);
		ImGui::Text("yMin             (2 bytes) : %hi", yMin);
		ImGui::Text("xMax             (2 bytes) : %hi", xMax);
		ImGui::Text("yMax             (2 bytes) : %hi", yMax);

		vWidgetId = simpleGlyph.draw(vWidgetId);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::glyfStruct::parse(MemoryStream *vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		numberOfContours = (uint16_t)vMem->ReadUShort();
		xMin = vMem->ReadFWord();
		yMin = vMem->ReadFWord();
		xMax = vMem->ReadFWord();
		yMax = vMem->ReadFWord();

		if (numberOfContours >= 0)
		{
			simpleGlyph.parse(vMem, vMem->GetPos(), vLength - vMem->GetPos(), numberOfContours);
		}
		else // compound
		{

		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::glyfTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("glyf Table :"))
	{
		for (auto & it : glyfs)
		{
			vWidgetId = it.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::glyfTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t /*vLength*/)
{
	if (vMem && loca)
	{
		int offset = 0;
		int length = 0;
		for (auto & it : loca->offsets)
		{
			offset = it;
			
			glyfStruct glyf;
			glyf.parse(vMem, vOffset + offset, length);
			glyfs.push_back(glyf);

			length = offset;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::cmapSubTableF4Struct::draw(int vWidgetId)
{
	if (filled)
	{
		ImGui::PushID(++vWidgetId);

		if (ImGui::TreeNode("cmap Sub Table F0 :"))
		{
			ImGui::Text("format             (2 bytes) : %hu", format);
			ImGui::Text("length             (2 bytes) : %hu", length);
			ImGui::Text("language           (2 bytes) : %hu", language);
			ImGui::Text("segCountX2         (2 bytes) : %hu", segCountX2);
			ImGui::Text("entrySelector      (2 bytes) : %hu", entrySelector);
			ImGui::Text("rangeShift         (2 bytes) : %hu", rangeShift);

			if (ImGui::TreeNode("endCode :"))
			{
				for (auto & it : endCode)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}
				
				ImGui::TreePop();
			}

			ImGui::Text("reservedPad        (2 bytes) : %hu", rangeShift);
			
			if (ImGui::TreeNode("startCode :"))
			{
				for (auto & it : startCode)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("idDelta :"))
			{
				for (auto & it : idDelta)
				{
					ImGui::Text("                   (2 bytes) : %hi", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("idRangeOffset :"))
			{
				for (auto & it : idRangeOffset)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("glyphIdArray :"))
			{
				for (auto & it : glyphIdArray)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	
	return vWidgetId;
}

void FontAnalyser::cmapSubTableF4Struct::parse(MemoryStream *vMem, size_t /*vOffset*/, size_t /*vLength*/)
{
	if (vMem)
	{
		filled = true;

		language = (uint16_t)vMem->ReadUShort();
		segCountX2 = (uint16_t)vMem->ReadUShort();
		searchRange = (uint16_t)vMem->ReadUShort();
		entrySelector = (uint16_t)vMem->ReadUShort();
		rangeShift = (uint16_t)vMem->ReadUShort();

		int segCount = segCountX2 / 2;
		for (int i = 0; i < segCount; i++)
		{
			endCode.push_back((uint16_t)vMem->ReadUShort());
		}
		reservedPad = (uint16_t)vMem->ReadUShort();
		for (int i = 0; i < segCount; i++)
		{
			startCode.push_back((uint16_t)vMem->ReadUShort());
		}
		for (int i = 0; i < segCount; i++)
		{
			idDelta.push_back((uint16_t)vMem->ReadShort());
		}
		for (int i = 0; i < segCount; i++)
		{
			idRangeOffset.push_back((uint16_t)vMem->ReadUShort());
		}

		//std::vector<uint16_t> glyphIdArray;
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
			
			if (ImGui::TreeNode("glyphIndexArray :"))
			{
				for (auto & it : glyphIndexArray)
				{
					ImGui::Text("(1 byte) : %hu", it);
				}

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	return vWidgetId;
}

void FontAnalyser::cmapSubTableF0Struct::parse(MemoryStream *vMem, size_t /*vOffset*/, size_t /*vLength*/)
{
	if (vMem)
	{
		filled = true;
		language = (uint16_t)vMem->ReadUShort();

		for (int i = 0; i < 256; i++)
		{
			glyphIndexArray[i] = vMem->ReadByte();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::cmapEncodingRecordStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("cmap SubTable"))
	{
		ImGui::Text("platformID         (2 bytes) : %hu", platformID);
		ImGui::Text("platformSpecificID (2 bytes) : %hu", encodingID);
		ImGui::Text("offset             (4 bytes) : %u", offset);
		
		vWidgetId = subTableF0.draw(vWidgetId);
		vWidgetId = subTableF4.draw(vWidgetId);

		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::cmapEncodingRecordStruct::parse(MemoryStream *vMem, size_t /*vOffset*/, size_t /*vLength*/)
{
	if (vMem)
	{
		platformID = (uint16_t)vMem->ReadUShort();
		encodingID = (uint16_t)vMem->ReadUShort();
		offset = (uint16_t)vMem->ReadULong();

		uint16_t format = (uint16_t)vMem->ReadUShort();
		uint16_t length = (uint16_t)vMem->ReadUShort();

		if (format == 0)
		{
			subTableF0.format = format;
			subTableF0.length = length;
			subTableF0.parse(vMem, vMem->GetPos(), length);
		}
		else if (format == 4)
		{
			subTableF4.format = format;
			subTableF4.length = length;
			subTableF4.parse(vMem, vMem->GetPos(), length);
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

	if (ImGui::TreeNode("cmap Table :"))
	{
		ImGui::Text("version            (2 bytes) : %hu", version);
		ImGui::Text("numEncodingRecords (2 bytes) : %hu", numEncodingRecords);

		for (auto & it : encodingRecords)
		{
			vWidgetId = it.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::cmapTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t /*vLength*/)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = (uint16_t)vMem->ReadUShort();
		numEncodingRecords = (uint16_t)vMem->ReadUShort();

		for (int i = 0; i < numEncodingRecords; i++)
		{
			cmapEncodingRecordStruct enc;
			enc.parse(vMem, 0, 0);
			encodingRecords.push_back(enc);
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

void FontAnalyser::headTableStruct::parse(MemoryStream *vMem, size_t vOffset, size_t /*vLength*/)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = vMem->ReadFixed();
		fontRevision = vMem->ReadFixed();
		checkSumAdjustment = (uint32_t)vMem->ReadULong();
		magicNumber = (uint32_t)vMem->ReadULong();
		flags = (uint16_t)vMem->ReadUShort(); // bitset
		unitsPerEm = (uint16_t)vMem->ReadUShort();
		created = vMem->ReadDateTime();
		modified = vMem->ReadDateTime();
		xMin = vMem->ReadFWord();
		yMin = vMem->ReadFWord();
		xMax = vMem->ReadFWord();
		yMax = vMem->ReadFWord();
		macStyle = (uint16_t)vMem->ReadUShort(); // bitset
		lowestRecPPEM = (uint16_t)vMem->ReadUShort();
		fontDirectionHint = (int16_t)vMem->ReadShort();
		indexToLocFormat = (int16_t)vMem->ReadShort();
		glyphDataFormat = (int16_t)vMem->ReadShort();
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
		numberOfGlyphs = (uint16_t)vMem->ReadUShort();
		if (numberOfGlyphs)
		{
			for (int i = 0; i < numberOfGlyphs; i++)
			{
				glyphNameIndex.push_back((uint16_t)vMem->ReadUShort());
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
		underlinePosition = (int16_t)vMem->ReadUShort();
		underlineThickness = (int16_t)vMem->ReadUShort();
		isFixedPitch = (uint32_t)vMem->ReadULong();
		minMemType42 = (uint32_t)vMem->ReadULong();
		maxMemType42 = (uint32_t)vMem->ReadULong();
		minMemType1 = (uint32_t)vMem->ReadULong();
		maxMemType1 = (uint32_t)vMem->ReadULong();

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
			else if (it.first == "glyf") vWidgetId = glyf.draw(vWidgetId);
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
		IF_TABLE("loca")
		{
			IF_TABLE("head") IF_TABLE("maxp")
			{
				loca.head = &head;
				loca.maxp = &maxp;
				PARSE_TABLE("loca", loca);
			}
			IF_TABLE("head")
			{
				glyf.loca = &loca;
				PARSE_TABLE("glyf", glyf);
			}
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
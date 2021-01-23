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
#include <ctools/cTools.h>

using namespace FontAnalyser;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void TableDisplay::AddItem(std::string vItem, std::string vSize, std::string vInfos)
{
	std::vector<std::string> arr;
	arr.push_back(vItem);
	arr.push_back(vSize);
	arr.push_back(vInfos);
	array.push_back(arr);
}
void TableDisplay::DisplayTable(const char* vTableLabel, size_t vMaxCount)
{
	ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders;
	ImVec2 Size = ImVec2(-FLT_MIN, 0);
	if (vMaxCount)
	{
		Size.y = ImGui::GetTextLineHeightWithSpacing() * (vMaxCount + 1);
		flags |= ImGuiTableFlags_ScrollY;
	}

	if (ImGui::BeginTable(vTableLabel, 3, flags, Size))
	{
		ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
		ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, -1, 0);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, -1, 1);
		ImGui::TableSetupColumn("Infos", ImGuiTableColumnFlags_WidthStretch, -1, 1);
		ImGui::TableHeadersRow(); // draw headers

		m_Clipper.Begin((int)array.size(), ImGui::GetTextLineHeightWithSpacing());
		while (m_Clipper.Step())
		{
			for (int i = m_Clipper.DisplayStart; i < m_Clipper.DisplayEnd; i++)
			{
				if (i < 0) continue;

				auto line = array[i];

				ImGui::TableNextRow();
				int idx = 0;
				for (auto column : line)
				{
					if (idx > 2) break; //3 columns max
					if (ImGui::TableSetColumnIndex(idx++))
					{
						ImGui::Text("%s", column.c_str());
					}
				}
			}
		}

		ImGui::EndTable();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::HeaderStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Header"))
	{
		DisplayTable("Header");

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

		AddItem("scalerType", "(4 bytes)", ct::toStr("% s", (scalerType[0] == 0 ? (scalerType[1] == 1 ? "TRUE" : "") : scalerType.c_str())));
		AddItem("numTables", "(2 bytes)", ct::toStr("%i", numTables));
		AddItem("searchRange", "(2 bytes)", ct::toStr("%i", searchRange));
		AddItem("entrySelector", "(2 bytes)", ct::toStr("%i", entrySelector));
		AddItem("rangeShift", "(4 bytes)", ct::toStr("%i", rangeShift));
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
		DisplayTable("Table s");

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

		AddItem("tag", "(4 bytes)", ct::toStr("%s", tag));
		AddItem("checkSum", "(4 bytes)", ct::toStr("%i", checkSum));
		AddItem("offset", "(4 bytes)", ct::toStr("%i", offset));
		AddItem("length", "(4 bytes)", ct::toStr("%i", length));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::NameRecord::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("name Record :", "name Record : %s", name.c_str()))
	{
		DisplayTable("name Record");

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::NameRecord::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		platformID = (uint16_t)vMem->ReadUShort();
		encodingID = (uint16_t)vMem->ReadUShort();
		languageID = (uint16_t)vMem->ReadUShort();
		nameID = (uint16_t)vMem->ReadUShort();
		length = (uint16_t)vMem->ReadUShort();
		stringOffset = (uint16_t)vMem->ReadUShort();

		AddItem("platformID", "(2 bytes)", ct::toStr("%u", platformID));
		AddItem("encodingID", "(2 bytes)", ct::toStr("%u", encodingID));
		AddItem("languageID", "(2 bytes)", ct::toStr("%u", languageID));
		AddItem("nameID", "(2 bytes)", ct::toStr("%u", nameID));
		AddItem("length", "(2 bytes)", ct::toStr("%u", length));
		AddItem("stringOffset", "(2 bytes)", ct::toStr("%u", stringOffset));
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::nameTableV0Struct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("name Table V0 :"))
	{
		DisplayTable("name Table V0");

		for (size_t i = 0; i < count; i++)
		{
			vWidgetId = nameRecords[i].draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::nameTableV0Struct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		count = (uint16_t)vMem->ReadUShort();
		storageOffset = (uint16_t)vMem->ReadUShort();

		nameRecords.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			NameRecord nr;
			nr.parse(vMem, vOffset + 4U + nr.GetSizeof() * i, nr.GetSizeof());
			nameRecords[i] = nr;
		}

		for (auto& nr : nameRecords)
		{
			if (nr.length)
			{
				vMem->SetPos(vOffset - 2U + storageOffset + nr.stringOffset); // -2U car storageOffset est depuis le debut de la table, et il a le champ version qui dans nametable
				nr.name = vMem->ReadString(nr.length);
			}
		}

		AddItem("count", "(2 bytes)", ct::toStr("%u", count));
		AddItem("storageOffset", "(2 bytes)", ct::toStr("%u", storageOffset));
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::LangTagRecordStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Lang Tag Record :"))
	{
		DisplayTable("Lang Tag Record");

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::LangTagRecordStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::nameTableV1Struct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("name Table V1 :"))
	{
		DisplayTable("name Table V1");

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::nameTableV1Struct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::nameTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("name Table :"))
	{
		DisplayTable("name Table");

		if (version == 0)
		{
			vWidgetId = nameTableV0.draw(vWidgetId);
		}
		else
		{
			vWidgetId = nameTableV1.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::nameTableStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = (uint16_t)vMem->ReadUShort(); // 2U

		if (version == 0)
		{
			nameTableV0.parse(vMem, vOffset + 2U, vLength - 2U);
		}
		else
		{
			nameTableV1.parse(vMem, vOffset + 2U, vLength - 2U);
		}

		AddItem("version", "(2 bytes)", ct::toStr("%u", version));
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
		DisplayTable("maxp Table");

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

		AddItem("version","(4 bytes)",ct::toStr("%hi.%hi", version.high, version.low));
		AddItem("numGlyphs", "(2 bytes)",ct::toStr("%hu", numGlyphs));
		AddItem("maxPoints", "(2 bytes)",ct::toStr("%hu", maxPoints));
		AddItem("maxContours", "(2 bytes)",ct::toStr("%hu", maxContours));
		AddItem("maxComponentPoints", "(2 bytes)",ct::toStr("%hu", maxComponentPoints));
		AddItem("maxComponentContours", "(2 bytes)",ct::toStr("%hu", maxComponentContours));
		AddItem("maxZones", "(2 bytes)",ct::toStr("%hu", maxZones));
		AddItem("maxTwilightPoints", "(2 bytes)",ct::toStr("%hu", maxTwilightPoints));
		AddItem("maxStorage", "(2 bytes)",ct::toStr("%hu", maxStorage));
		AddItem("maxFunctionDefs", "(2 bytes)",ct::toStr("%hu", maxFunctionDefs));
		AddItem("maxInstructionDefs", "(2 bytes)",ct::toStr("%hu", maxInstructionDefs));
		AddItem("maxStackElements", "(2 bytes)",ct::toStr("%hu", maxStackElements));
		AddItem("maxSizeOfInstructions", "(2 bytes)",ct::toStr("%hu", maxSizeOfInstructions));
		AddItem("maxComponentElements", "(2 bytes)",ct::toStr("%hu", maxComponentElements));
		AddItem("maxComponentDepth", "(2 bytes)",ct::toStr("%hu", maxComponentDepth));
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
		DisplayTable("loca Table", 20);

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::locaTableStruct::parse(MemoryStream* vMem, size_t vOffset, size_t /*vLength*/)
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

		if (head->indexToLocFormat == 0) // short format
		{
			for (auto& it : offsets)
			{
				AddItem("offsets", "(2 bytes)", ct::toStr("%hu", it));
			}
		}
		else if (head->indexToLocFormat == 1) // long format
		{
			for (auto& it : offsets)
			{
				AddItem("offsets", "(4 bytes)", ct::toStr("%u", it));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::baseGlyphRecordStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Base Glyph Record :"))
	{
		DisplayTable("Base Glyph Record");

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::baseGlyphRecordStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		glyphID = (uint16_t)vMem->ReadUShort();
		firstLayerIndex = (uint16_t)vMem->ReadUShort();
		numLayers = (uint16_t)vMem->ReadUShort();

		AddItem("glyphID", "(2 bytes)", ct::toStr("%hu", glyphID));
		AddItem("firstLayerIndex", "(2 bytes)", ct::toStr("%hu", firstLayerIndex));
		AddItem("numLayers", "(2 bytes)", ct::toStr("%hu", numLayers));
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::layerRecordStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Layer Record :"))
	{
		DisplayTable("Layer Record");
		
		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::layerRecordStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		glyphID = (uint16_t)vMem->ReadUShort();
		paletteIndex = (uint16_t)vMem->ReadUShort();

		AddItem("glyphID", "(2 bytes)", ct::toStr("%hu", glyphID));
		AddItem("paletteIndex", "(2 bytes)", ct::toStr("%hu", paletteIndex));
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::colrTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("COLR Table :"))
	{
		DisplayTable("COLR Table");

		if (ImGui::TreeNode("Glyph Records :", "Glyph Records : %u", numBaseGlyphRecords))
		{
			if (!baseGlyphRecords.empty())
			{
				ImGuiListClipper clipper;
				clipper.Begin((int)baseGlyphRecords.size(), ImGui::GetTextLineHeightWithSpacing());
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						vWidgetId = baseGlyphRecords[i].draw(vWidgetId);
					}
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Layer Records :", "Layer Records : %u", numLayerRecords))
		{
			if (!layerRecords.empty())
			{
				ImGuiListClipper clipper;
				clipper.Begin((int)layerRecords.size(), ImGui::GetTextLineHeightWithSpacing());
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						if (i < 0) continue;

						vWidgetId = layerRecords[i].draw(vWidgetId);
					}
				}
			}

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void FontAnalyser::colrTableStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = (uint16_t)vMem->ReadUShort();
		numBaseGlyphRecords = (uint16_t)vMem->ReadUShort();
		baseGlyphRecordsOffset = (uint32_t)vMem->ReadULong();
		layerRecordsOffset = (uint32_t)vMem->ReadULong();
		numLayerRecords = (uint16_t)vMem->ReadUShort();

		AddItem("version", "(2 bytes)", ct::toStr("%hu", version));
		AddItem("numBaseGlyphRecords", "(2 bytes)", ct::toStr("%hu", numBaseGlyphRecords));
		AddItem("baseGlyphRecordsOffset", "(4 bytes)", ct::toStr("%hu", baseGlyphRecordsOffset));
		AddItem("layerRecordsOffset", "(4 bytes)", ct::toStr("%hu", layerRecordsOffset));
		AddItem("numLayerRecords", "(2 bytes)", ct::toStr("%hu", numLayerRecords));

		for (int i = 0; i < numBaseGlyphRecords; i++)
		{
			baseGlyphRecordStruct base;
			base.parse(vMem, vOffset + baseGlyphRecordsOffset + base.GetSizeof() * i, base.GetSizeof());
			baseGlyphRecords.push_back(base);
		}

		for (int i = 0; i < numLayerRecords; i++)
		{
			layerRecordStruct layer;
			layer.parse(vMem, vOffset + layerRecordsOffset + layer.GetSizeof() * i, layer.GetSizeof());
			layerRecords.push_back(layer);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

int colorRecordStruct::draw(int vWidgetId)
{
	ImGui::ColorButton("##color", color);

	return vWidgetId;
}

void colorRecordStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		blue = vMem->ReadByte();
		green = vMem->ReadByte();
		red = vMem->ReadByte();
		alpha = vMem->ReadByte();

		color = ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
	}
}

///////////////////////////////////////////////////////////////////////////////

int paletteStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Palette :"))
	{
		int idx = 0;
		for (auto& c : colorRecords)
		{
			if (idx++ > 0 && (idx % 30) != 0)
				ImGui::SameLine();
			vWidgetId = c.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void paletteStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vOffset);
	UNUSED(vLength);

	if (vMem)
	{
		
	}
}

///////////////////////////////////////////////////////////////////////////////

int cpalTableV0Struct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("CPAL v0 Table :"))
	{
		DisplayTable("CPAL v0 Table");

		for (auto& p : palettes)
		{
			vWidgetId = p.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void cpalTableV0Struct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		numPaletteEntries = (uint16_t)vMem->ReadUShort();
		numPalettes = (uint16_t)vMem->ReadUShort();
		numColorRecords = (uint16_t)vMem->ReadUShort();
		colorRecordsArrayOffset = (uint32_t)vMem->ReadULong();

		AddItem("numPaletteEntries", "(2 bytes)", ct::toStr("%hu", numPaletteEntries));
		AddItem("numPalettes", "(2 bytes)", ct::toStr("%hu", numPalettes));
		AddItem("numColorRecords", "(4 bytes)", ct::toStr("%hu", numColorRecords));
		AddItem("colorRecordsArrayOffset", "(4 bytes)", ct::toStr("%hu", colorRecordsArrayOffset));
		
		colorRecordIndices.resize(numPalettes);
		palettes.resize(numPalettes);
		
		for (int paletteIndex = 0; paletteIndex < numPalettes; paletteIndex++)
		{
			colorRecordIndices[paletteIndex] = (uint16_t)vMem->ReadUShort();
		}

		for (int paletteIndex = 0; paletteIndex < numPalettes; paletteIndex++)
		{
			for (int paletteEntryIndex = 0; paletteEntryIndex < numPaletteEntries; paletteEntryIndex++)
			{
				colorRecordStruct col;
				
				uint32_t colorRecordOffset = colorRecordIndices[paletteIndex] + paletteEntryIndex;

				col.parse(vMem, vOffset - 2U + colorRecordsArrayOffset + col.GetSizeof() * colorRecordOffset, col.GetSizeof()); // -2U car il ya version au debut de la table
				palettes[paletteIndex].colorRecords.push_back(col);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

int cpalTableV1Struct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("CPAL v1 Table :"))
	{
		DisplayTable("CPAL v1 Table");

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void cpalTableV1Struct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	UNUSED(vLength);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		numPaletteEntries = (uint16_t)vMem->ReadUShort();
		numPalettes = (uint16_t)vMem->ReadUShort();
		numColorRecords = (uint16_t)vMem->ReadUShort();
		colorRecordsArrayOffset = (uint32_t)vMem->ReadULong();

		vMem->SetPos(vMem->GetPos() + sizeof(uint16_t) * numPalettes);
		/*for (int i = 0; i < numPalettes; i++)
		{
			vMem->SetPos(vMem->GetPos() + sizeof(uint16_t) * numPalettes)
			//baseGlyphRecordStruct base;
			//base.parse(vMem, vOffset - 2U + baseGlyphRecordsOffset + sizeof(baseGlyphRecordStruct) * i, sizeof(baseGlyphRecordStruct)); // -2U car il y a version en debut de table
			//baseGlyphRecords.push_back(base);
		}*/

		paletteTypesArrayOffset = (uint32_t)vMem->ReadULong();
		paletteLabelsArrayOffset = (uint32_t)vMem->ReadULong();
		paletteEntryLabelsArrayOffset = (uint32_t)vMem->ReadULong();

		AddItem("numPaletteEntries", "(2 bytes)", ct::toStr("%hu", numPaletteEntries));
		AddItem("numPalettes", "(2 bytes)", ct::toStr("%hu", numPalettes));
		AddItem("numColorRecords", "(2 bytes)", ct::toStr("%hu", numColorRecords));
		AddItem("colorRecordsArrayOffset", "(4 bytes)", ct::toStr("%hu", colorRecordsArrayOffset));
		AddItem("paletteTypesArrayOffset", "(2 bytes)", ct::toStr("%hu", paletteTypesArrayOffset));
		AddItem("paletteLabelsArrayOffset", "(4 bytes)", ct::toStr("%hu", paletteLabelsArrayOffset));
		AddItem("paletteEntryLabelsArrayOffset", "(4 bytes)", ct::toStr("%hu", paletteEntryLabelsArrayOffset));
	}
}

///////////////////////////////////////////////////////////////////////////////

int cpalTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("CPAL Table :"))
	{
		DisplayTable("CPAL Table");

		if (version == 0U)
		{
			vWidgetId = tableV0Struct.draw(vWidgetId);
		}
		else if (version == 1U)
		{
			vWidgetId = tableV1Struct.draw(vWidgetId);
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	ImGui::Separator();

	return vWidgetId;
}

void cpalTableStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength)
{
	if (vMem)
	{
		vMem->SetPos(vOffset);

		version = (uint16_t)vMem->ReadUShort();
		
		AddItem("version", "(2 bytes)", ct::toStr("%hu", version));
		
		if (version == 0U)
		{
			tableV0Struct.parse(vMem, vOffset + 2U, vLength - 2U);
		}
		else if (version == 1U)
		{
			tableV1Struct.parse(vMem, vOffset + 2U, vLength - 2U);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::compositeGlyphTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("Composite Glyph :"))
	{
		ImGui::TreePop();
	}

	ImGui::PopID();

	return vWidgetId;
}

void FontAnalyser::compositeGlyphTableStruct::parse(MemoryStream* vMem, size_t vOffset, size_t vLength, int16_t vCountContours)
{
	UNUSED(vLength);
	UNUSED(vCountContours);

	if (vMem)
	{
		vMem->SetPos(vOffset);

		flags = (uint16_t)vMem->ReadUShort();
		glyphIndex = (uint16_t)vMem->ReadUShort();

		if (flags & ARG_1_AND_2_ARE_WORDS)
		{
			argument1_16 = (uint16_t)vMem->ReadUShort();
			argument2_16 = (uint16_t)vMem->ReadUShort();
		}
		else
		{
			argument1_8 = (uint16_t)vMem->ReadByte();
			argument2_8 = (uint16_t)vMem->ReadByte();
		}
		if (flags & WE_HAVE_A_SCALE)
		{
			scale = vMem->ReadF2DOT14();
		}
		else if (flags & WE_HAVE_AN_X_AND_Y_SCALE)
		{
			xscale = vMem->ReadF2DOT14();
			yscale = vMem->ReadF2DOT14();
		}
		else if (flags & WE_HAVE_A_TWO_BY_TWO)
		{
			xscale = vMem->ReadF2DOT14();
			scale01 = vMem->ReadF2DOT14();
			scale10 = vMem->ReadF2DOT14();
			yscale = vMem->ReadF2DOT14();
		}
		/*while (flags & MORE_COMPONENTS)
		{
			if (flags & WE_HAVE_INSTRUCTIONS)
			{
				uint16 numInstr
				uint8 instr[numInstr]
			}
		}*/
	}
}

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
				for (auto& it : endPtsOfContours)
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
					for (auto& it : endPtsOfContours)
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
				for (auto& it : flags)
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
				for (auto& it : xCoordinates)
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
				for (auto& it : yCoordinates)
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

int FontAnalyser::glyfStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("glyf :"))
	{
		DisplayTable("glyf");

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

		numberOfContours = (int16_t)vMem->ReadShort();
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
			compositeGlyph.parse(vMem, vMem->GetPos(), vLength - vMem->GetPos(), numberOfContours);
		}

		AddItem("numberOfContours", "(2 bytes)", ct::toStr("%hu", numberOfContours));
		AddItem("xMin", "(2 bytes)", ct::toStr("%hi", xMin));
		AddItem("yMin", "(2 bytes)", ct::toStr("%hi", yMin));
		AddItem("xMax", "(2 bytes)", ct::toStr("%hi", xMax));
		AddItem("yMax", "(2 bytes)", ct::toStr("%hi", yMax));
	}
}

///////////////////////////////////////////////////////////////////////////////

int FontAnalyser::glyfTableStruct::draw(int vWidgetId)
{
	ImGui::PushID(++vWidgetId);

	if (ImGui::TreeNode("glyf Table :"))
	{
		if (!glyfs.empty())
		{
			ImGuiListClipper clipper;
			clipper.Begin((int)glyfs.size(), ImGui::GetTextLineHeightWithSpacing());
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					if (i < 0) continue;

					vWidgetId = glyfs[i].draw(vWidgetId);
				}
			}
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

		if (ImGui::TreeNode("cmap Sub Table F4 :"))
		{
			DisplayTable("cmap Sub Table F4");

			if (ImGui::TreeNode("endCode :"))
			{
				for (auto& it : endCode)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			ImGui::Text("reservedPad        (2 bytes) : %hu", rangeShift);

			if (ImGui::TreeNode("startCode :"))
			{
				for (auto& it : startCode)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("idDelta :"))
			{
				for (auto& it : idDelta)
				{
					ImGui::Text("                   (2 bytes) : %hi", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("idRangeOffset :"))
			{
				for (auto& it : idRangeOffset)
				{
					ImGui::Text("                   (2 bytes) : %hu", it);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("glyphIdArray :"))
			{
				for (auto& it : glyphIdArray)
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

		AddItem("format", "(2 bytes)", ct::toStr("%hu", format));
		AddItem("length", "(2 bytes)", ct::toStr("%hu", length));
		AddItem("language", "(2 bytes)", ct::toStr("%hu", language));
		AddItem("segCountX2", "(2 bytes)", ct::toStr("%hu", segCountX2));
		AddItem("entrySelector", "(2 bytes)", ct::toStr("%hu", entrySelector));
		AddItem("rangeShift", "(2 bytes)", ct::toStr("%hu", rangeShift));
		
		int segCount = segCountX2 / 2;
		for (int i = 0; i < segCount; i++)
		{
			endCode.push_back((uint16_t)vMem->ReadUShort());
		}
		reservedPad = (uint16_t)vMem->ReadUShort();
		AddItem("reservedPad", "(2 bytes)", ct::toStr("%hu", reservedPad));
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
			DisplayTable("cmap Sub Table F0");

			if (ImGui::TreeNode("glyphIndexArray :"))
			{
				for (auto& it : glyphIndexArray)
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

		AddItem("format", "(2 bytes)", ct::toStr("%hu", format));
		AddItem("length", "(2 bytes)", ct::toStr("%hu", length));
		AddItem("language", "(2 bytes)", ct::toStr("%hu", language));
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
		DisplayTable("cmap Sub Table");

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

		AddItem("platformID", "(2 bytes)", ct::toStr("%hu", platformID));
		AddItem("platformSpecificID", "(2 bytes)", ct::toStr("%hu", encodingID));
		AddItem("offset", "(4 bytes)", ct::toStr("%u", offset));

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
		DisplayTable("cmap Table");

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

		AddItem("version", "(2 bytes)", ct::toStr("%hu", version));
		AddItem("numEncodingRecords", "(2 bytes)", ct::toStr("%hu", numEncodingRecords));

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
		DisplayTable("head Table");

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

		AddItem("version", "(4 bytes)", ct::toStr("%hi.%hi", version.high, version.low));
		AddItem("fontRevision", "(4 bytes)", ct::toStr("%hi.%hi", fontRevision.high, fontRevision.low));
		AddItem("checkSumAdjustment", "(4 bytes)", ct::toStr("%u", checkSumAdjustment));
		AddItem("magicNumber", "(4 bytes)", ct::toStr("%X%s", magicNumber, (magicNumber == 0x5F0F3CF5 ? " (Valid)" : " (Not Valid)")));
		AddItem("flags", "(2 bytes)", ct::toStr("%hu", flags)); // bitset
		AddItem("unitsPerEm", "(2 bytes)", ct::toStr("%hu", unitsPerEm));
		AddItem("created", "(8 bytes)", ct::toStr("%lli", created));
		AddItem("modified", "(8 bytes)", ct::toStr("%lli", modified));
		AddItem("xMin", "(2 bytes)", ct::toStr("%hi", xMin));
		AddItem("yMin", "(2 bytes)", ct::toStr("%hi", yMin));
		AddItem("xMax", "(2 bytes)", ct::toStr("%hi", xMax));
		AddItem("yMax", "(2 bytes)", ct::toStr("%hi", yMax));
		AddItem("macStyle", "(2 bytes)", ct::toStr("%hu", macStyle)); // bitset
		AddItem("lowestRecPPEM", "(2 bytes)", ct::toStr("%hu", lowestRecPPEM));
		AddItem("fontDirectionHint", "(2 bytes)", ct::toStr("%hi", fontDirectionHint));
		AddItem("indexToLocFormat", "(2 bytes)", ct::toStr("%hi", indexToLocFormat));
		AddItem("glyphDataFormat", "(2 bytes)", ct::toStr("%hi", glyphDataFormat));
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
			DisplayTable("head Table");

			if (ImGui::TreeNode("glyphNameIndexs (2 bytes array of numberOfGlyphs)"))
			{
				for (auto& it : glyphNameIndex)
					ImGui::Text("glyph id        (2 bytes) : %hu", it);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("names :"))
			{
				for (auto& it : names)
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
		
		AddItem("numberOfGlyphs", "(2 bytes)", ct::toStr("%hu", numberOfGlyphs));

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
		DisplayTable("post Table");

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

		AddItem("format", "(4 bytes)", ct::toStr("%hi.%hi", format.high, format.low));
		AddItem("italicAngle", "(4 bytes)", ct::toStr("%hi.%hi", italicAngle.high, italicAngle.low));
		AddItem("underlinePosition", "(2 bytes)", ct::toStr("%hi", underlinePosition));
		AddItem("underlineThickness", "(2 bytes)", ct::toStr("%hi", underlineThickness));
		AddItem("isFixedPitch", "(4 bytes)", ct::toStr("%u", isFixedPitch));
		AddItem("minMemType42", "(4 bytes)", ct::toStr("%u", minMemType42));
		AddItem("maxMemType42", "(4 bytes)", ct::toStr("%u", maxMemType42));
		AddItem("minMemType1", "(4 bytes)", ct::toStr("%u", minMemType1));
		AddItem("maxMemType1", "(4 bytes)", ct::toStr("%u", maxMemType1));

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
#define DRAW_TABLE(_tag_, _class_) if (it.first == _tag_) vWidgetId = _class_.draw(vWidgetId)
#define ELSE_DRAW_TABLE(_tag_, _class_) else DRAW_TABLE(_tag_, _class_)

		for (auto & it : tables)
		{
			DRAW_TABLE("head", head);
			ELSE_DRAW_TABLE("name", name);
			ELSE_DRAW_TABLE("maxp", maxp);
			ELSE_DRAW_TABLE("cmap", cmap);
			ELSE_DRAW_TABLE("loca", loca);
			ELSE_DRAW_TABLE("post", post);
			ELSE_DRAW_TABLE("glyf", glyf);
			ELSE_DRAW_TABLE("COLR", colr);
			ELSE_DRAW_TABLE("CPAL", cpal);
		}

#undef DRAW_TABLE
#undef ELSE_DRAW_TABLE
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
		IF_TABLE("name") PARSE_TABLE("name", name);
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
		IF_TABLE("COLR") PARSE_TABLE("COLR", colr);
		IF_TABLE("CPAL") PARSE_TABLE("CPAL", cpal);

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
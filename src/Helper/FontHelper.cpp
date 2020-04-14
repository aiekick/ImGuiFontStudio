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

 // Writable font data wrapper. Supports reading of data primitives in the
 // TrueType / OpenType spec.
 // The data types used are as listed:
 // BYTE       8-bit unsigned integer.
 // CHAR       8-bit signed integer.
 // USHORT     16-bit unsigned integer.
 // SHORT      16-bit signed integer.
 // UINT24     24-bit unsigned integer.
 // ULONG      32-bit unsigned integer.
 // LONG       32-bit signed integer.
 // Fixed      32-bit signed fixed-point number (16.16)
 // FUNIT      Smallest measurable distance in the em space.
 // FWORD      16-bit signed integer (SHORT) that describes a quantity in FUnits.
 // UFWORD     16-bit unsigned integer (USHORT) that describes a quantity in
 //            FUnits.
 // F2DOT14    16-bit signed fixed number with the low 14 bits of fraction (2.14)
 // LONGDATETIME  Date represented in number of seconds since 12:00 midnight,
 //               January 1, 1904. The value is represented as a signed 64-bit
 //               integer.

#include "FontHelper.h"
#include <FileHelper.h>
#include <cTools.h>

#include <stdio.h>

#include <set>
#include <map>
#include <sstream>

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/data/font_data.h" // data type size
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/port/file_input_stream.h"
#include "sfntly/table/truetype/loca_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/core/maximum_profile_table.h"
#include "sfntly/table/core/post_script_table.h"
#include "sfntly/table/core/horizontal_header_table.h"
#include "sfntly/table/core/horizontal_metrics_table.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FontHelper::FontHelper()
{
	m_InvertedStandardNames = InvertNameMap();
}

FontHelper::~FontHelper()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool FontHelper::OpenFontFile(
	const std::string& vFontFilePathName, 
	std::map<CodePoint, std::string> vNewNames,
	std::map<CodePoint, CodePoint> vNewCodePoints,
	std::map<CodePoint, GlyphInfos> vNewGlyphInfos,
	bool vBaseFontFileToMergeIn)
{
	bool res = false;

	if (FileHelper::Instance()->IsFileExist(vFontFilePathName))
	{
		FontInstance fontInstance;

		fontInstance.m_Font.Attach(LoadFontFile(vFontFilePathName.c_str()));
		if (fontInstance.m_Font)
		{
			sfntly::Ptr<sfntly::CMapTable> cmap_table = down_cast<sfntly::CMapTable*>(fontInstance.m_Font->GetTable(sfntly::Tag::cmap));
			fontInstance.m_CMapTable.Attach(cmap_table->GetCMap(sfntly::CMapTable::WINDOWS_BMP));
			if (fontInstance.m_CMapTable)
			{
				fontInstance.m_GlyfTable = down_cast<sfntly::GlyphTable*>(fontInstance.m_Font->GetTable(sfntly::Tag::glyf));
				fontInstance.m_LocaTable = down_cast<sfntly::LocaTable*>(fontInstance.m_Font->GetTable(sfntly::Tag::loca));

				if (fontInstance.m_GlyfTable && fontInstance.m_LocaTable)
				{
					fontInstance.m_NewGlyphNames = vNewNames;
					fontInstance.m_NewGlyphCodePoints = vNewCodePoints;
					fontInstance.m_NewGlyphInfos = vNewGlyphInfos;

					FillCharacterMap(&fontInstance, fontInstance.m_NewGlyphNames);
					FillResolvedCompositeGlyphs(&fontInstance, fontInstance.m_CharMap);

					if (vBaseFontFileToMergeIn)
						m_BaseFontIdx = m_Fonts.size();

					m_Fonts.push_back(fontInstance);

					res = true;
				}
			}
		}
	}

	return res;
}

bool FontHelper::GenerateFontFile(
	const std::string& vFontFilePathName, 
	bool vUsePostTable) // when merge mode, will deinf what is the basis font
{
	bool res = false;

	if (!m_Fonts.empty())
	{
		// en mode merge de plusieurs fonts
		// merge des codepoint, c'est la que c'est important d'avoir un codepoint unique
		// sinon il y aura moins de codepoint que de glyph et ca va foutre le brin
		int err = MergeCharacterMaps();
		if (!err)
		{
			sfntly::Ptr<sfntly::Font> newFont;
			newFont.Attach(AssembleFont(vUsePostTable));
			if (newFont)
			{
				auto ps = FileHelper::Instance()->ParsePathFileName(vFontFilePathName);
				if (ps.isOk)
				{
					std::string filePathName = ps.name + ".ttf";
					if (!ps.path.empty())
						filePathName = ps.path + FileHelper::Instance()->m_SlashType + filePathName;
					res = SerializeFont(filePathName.c_str(), newFont);
				}
			}
		}
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

FontInstance* FontHelper::GetBaseFontInstance() // in merge mode the baseFontInstance car be other than first font
{
	if (m_Fonts.size() > m_BaseFontIdx)
	{
		return &m_Fonts[m_BaseFontIdx];
	}
	return 0;
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_info.cc*/
void FontHelper::FillCharacterMap(FontInstance *vFontInstance, std::map<CodePoint, std::string> vSelection)
{
	if (vFontInstance)
	{
		vFontInstance->m_CharMap.clear();
		vFontInstance->m_ReversedCharMap.clear();

		if (!vFontInstance->m_CMapTable) return;

		sfntly::CMapTable::CMap::CharacterIterator* 
			character_iterator = vFontInstance->m_CMapTable->Iterator();
		if (!character_iterator) return;

		while (character_iterator->HasNext())
		{
			int32_t codepoint = character_iterator->Next();
			// retain only selection or ratain all is no selection
			if ((vSelection.find(codepoint) != vSelection.end()) || vSelection.empty()) // found
			{
				vFontInstance->m_CharMap[codepoint] = vFontInstance->m_CMapTable->GlyphId(codepoint);
				vFontInstance->m_ReversedCharMap[vFontInstance->m_CharMap[codepoint]] = codepoint;
			}
		}
		delete character_iterator;
	}
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_info.cc*/
void FontHelper::FillResolvedCompositeGlyphs(FontInstance *vFontInstance, std::map<CodePoint, int32_t> chars_to_glyph_ids)
{
	if (vFontInstance)
	{
		vFontInstance->m_ResolvedSet.clear();
		vFontInstance->m_ResolvedSet.insert(0); // why insert a 0 ?? java conversion mistake ??
		std::set<int32_t> unresolved_glyph_ids;

		// Since composite glyph elements might themselves be composite, we would need
		// to recursively resolve the elements too. To avoid the recursion we
		// create two sets, |unresolved_glyph_ids| for the unresolved glyphs,
		// initially containing all the ids and |resolved_glyph_ids|, initially empty.
		// We'll remove glyph ids from |unresolved_glyph_ids| until it is empty and,
		// if the glyph is composite, add its elements to the unresolved set.
		for (auto it = chars_to_glyph_ids.begin(),
			e = chars_to_glyph_ids.end(); it != e; ++it)
		{
			unresolved_glyph_ids.insert(it->second);
		}

		// As long as there are unresolved glyph ids.
		while (!unresolved_glyph_ids.empty())
		{
			// Get the corresponding glyph.
			int32_t glyph_id = *(unresolved_glyph_ids.begin());
			unresolved_glyph_ids.erase(unresolved_glyph_ids.begin());
			if (glyph_id < 0 || glyph_id > vFontInstance->m_LocaTable->num_glyphs())
			{
				fprintf(stderr, "%d larger than %d or smaller than 0\n", glyph_id,
					vFontInstance->m_LocaTable->num_glyphs());
				continue;
			}
			int32_t length = vFontInstance->m_LocaTable->GlyphLength(glyph_id);
			if (length == 0)
			{
				fprintf(stderr, "Zero length glyph %d\n", glyph_id);
				continue;
			}
			int32_t offset = vFontInstance->m_LocaTable->GlyphOffset(glyph_id);
			sfntly::GlyphPtr glyph;
			glyph.Attach(vFontInstance->m_GlyfTable->GetGlyph(offset, length));
			if (glyph == NULL)
			{
				fprintf(stderr, "GetGlyph returned NULL for %d\n", glyph_id);
				continue;
			}
			// Mark the glyph as resolved.
			vFontInstance->m_ResolvedSet.insert(glyph_id);
			// If it is composite, add all its components to the unresolved glyph set.
			if (glyph->GlyphType() == sfntly::GlyphType::kComposite)
			{
				sfntly::Ptr<sfntly::GlyphTable::CompositeGlyph> composite_glyph =
					down_cast<sfntly::GlyphTable::CompositeGlyph*>(glyph.p_);
				int32_t num_glyphs = composite_glyph->NumGlyphs();
				for (int32_t i = 0; i < num_glyphs; ++i)
				{
					glyph_id = composite_glyph->GlyphIndex(i);
					if (vFontInstance->m_ResolvedSet.find(glyph_id) == vFontInstance->m_ResolvedSet.end())
					{
						unresolved_glyph_ids.insert(glyph_id);
					}
				}
			}
		}
		unresolved_glyph_ids.clear();
	}

}

int32_t FontHelper::MergeCharacterMaps()
{
	m_CharMap.clear(); // codepoint to glyph id
	m_ReversedCharMap.clear(); // glyph id to codepoint
	m_ResolvedSet.clear();
	m_GlyphNames.clear();
	
	size_t fontId = 0;
	for (auto &font : m_Fonts)
	{
		for (auto &cm : font.m_CharMap)
		{
			if (font.m_NewGlyphCodePoints.find(cm.first) != font.m_NewGlyphCodePoints.end())
			{
				CodePoint newCodePoint = font.m_NewGlyphCodePoints[cm.first];
				m_CharMap[newCodePoint] = FontGlyphId(fontId, cm.second);
				m_ReversedCharMap[FontGlyphId(fontId, cm.second)] = newCodePoint;
				m_GlyphNames[newCodePoint] = font.m_NewGlyphNames[cm.first];
			}
			else
			{
				assert(0); // normally impossible to catch
			}
		}
		for (auto &rs : font.m_ResolvedSet)
		{
			m_ResolvedSet.emplace(FontGlyphId(fontId, rs));
		}

		fontId++;
	}

	return 0;
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_assembler.cc*/
sfntly::Font* FontHelper::AssembleFont(bool vUsePostTable)
//sfntly::Font* FontHelper::AssembleFont(std::set<int32_t> vTableBlackList) old signature
{
	auto fontInstance = GetBaseFontInstance();
	if (fontInstance)
	{
		if (m_Fonts.size() > 0)
		{
			m_FontFactory.Attach(sfntly::FontFactory::GetInstance());
			m_FontBuilder.Attach(m_FontFactory->NewFontBuilder());

			// Assemble tables
			bool CanWeGo = true;
			CanWeGo &= Assemble_Glyf_Loca_Maxp_Tables();
			CanWeGo &= Assemble_CMap_Table();
			CanWeGo &= Assemble_Hmtx_Hhea_Tables();
			CanWeGo &= Assemble_Meta_Table(); // not made for the moment
			CanWeGo &= Assemble_Head_Table(); // not made for the moment
			if (vUsePostTable)
				CanWeGo &= Assemble_Post_Table(m_GlyphNames);
			if (CanWeGo)
			{
				// include for the moment only head, before generate it
				// head table is needed else it will be a not loadable font
				const sfntly::TableMap* common_table_map = fontInstance->m_Font->GetTableMap();
				for (sfntly::TableMap::const_iterator it = common_table_map->begin(), e = common_table_map->end(); it != e; ++it)
				{
					//if (vTableBlackList.find(it->first) != vTableBlackList.end()) // found
					//	continue;
					if (it->second->header_tag() == sfntly::Tag::head) // a terme il faudra generer head
						m_FontBuilder->NewTableBuilder(it->first, it->second->ReadFontData());
				}

				return m_FontBuilder->Build();
			}
		}
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_assembler.cc*/
bool FontHelper::Assemble_Glyf_Loca_Maxp_Tables()
{
	auto baseFontInstance = GetBaseFontInstance();
	if (baseFontInstance)
	{
		m_OldToNewGlyfId.clear();
		m_NewToOldGlyfId.clear();

		sfntly::Ptr<sfntly::LocaTable::Builder> loca_table_builder = down_cast<sfntly::LocaTable::Builder*>(m_FontBuilder->NewTableBuilder(sfntly::Tag::loca));
		sfntly::Ptr<sfntly::GlyphTable::Builder> glyph_table_builder = down_cast<sfntly::GlyphTable::Builder*>(m_FontBuilder->NewTableBuilder(sfntly::Tag::glyf));
		sfntly::GlyphTable::GlyphBuilderList* glyph_builders = glyph_table_builder->GlyphBuilders();

		int32_t fontId = 0;
		int32_t new_glyphid = 0;
		for (auto it = m_ResolvedSet.begin(), e = m_ResolvedSet.end(); it != e; ++it)
		{
			// Get the glyph for this resolved_glyph_id.
			fontId = it->first;
			int32_t resolved_glyph_id = it->second;
			m_OldToNewGlyfId[*it] = new_glyphid++;
			m_NewToOldGlyfId[fontId].push_back(resolved_glyph_id);

			// we will need to scale the glyph contours here or somewhere for merging mode
			// bounding box cant be the sames between fonts

			// Get the LOCA table for the current glyph id.
			sfntly::Ptr<sfntly::LocaTable> loca_table = down_cast<sfntly::LocaTable*>(m_Fonts[fontId].m_Font->GetTable(sfntly::Tag::loca));
			int32_t length = loca_table->GlyphLength(resolved_glyph_id);
			int32_t offset = loca_table->GlyphOffset(resolved_glyph_id);

			// Get the GLYF table for the current glyph id.
			sfntly::Ptr<sfntly::GlyphTable> glyph_table = down_cast<sfntly::GlyphTable*>(m_Fonts[fontId].m_Font->GetTable(sfntly::Tag::glyf));
			sfntly::GlyphPtr glyph;
			glyph.Attach(glyph_table->GetGlyph(offset, length));

			// glyph readable
			sfntly::Ptr<sfntly::ReadableFontData> actualGlyfData = glyph->ReadFontData();

			////////////////////////////////////////////////////////////////////////////
			//// maybe i can edit copy_data before write in glyph_builder container ////
			////////////////////////////////////////////////////////////////////////////

			sfntly::Ptr<sfntly::WritableFontData> newGlyfTable = ReScale_Glyph(fontId, resolved_glyph_id, actualGlyfData);

			////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////

			// put in a glyphbuilder
			sfntly::GlyphBuilderPtr glyph_builder;
			glyph_builder.Attach(glyph_table_builder->GlyphBuilder(newGlyfTable));

			// put in total glyphs builder
			glyph_builders->push_back(glyph_builder);
		}

		sfntly::IntegerList loca_list;
		glyph_table_builder->GenerateLocaList(&loca_list);
		loca_table_builder->SetLocaList(&loca_list);

		sfntly::Ptr<sfntly::ReadableFontData> rFontData = baseFontInstance->m_Font->GetTable(sfntly::Tag::maxp)->ReadFontData();
		sfntly::Ptr<sfntly::MaximumProfileTable::Builder> maxpBuilder =
			down_cast<sfntly::MaximumProfileTable::Builder*>(m_FontBuilder->NewTableBuilder(sfntly::Tag::maxp, rFontData));

		maxpBuilder->SetNumGlyphs(loca_list.size() - 1);
		//maxpBuilder->SetNumGlyphs(m_ResolvedSet.size());
		//maxpBuilder->SetNumGlyphs(loca_table_builder->NumGlyphs());

		return true;
	}

	return false;
}

sfntly::Ptr<sfntly::WritableFontData> FontHelper::ReScale_Glyph(
	const int32_t& vFontId, const int32_t& vGlyphId,
	sfntly::Ptr<sfntly::ReadableFontData> vReadableFontData)
{
	// we will not add or remove points
	// just apply trasnofrmation soe the size will not change
	// so we will use vWritableFontData for overwrite datas if needed
	// easier way instead of regenerate glyph

	if (vReadableFontData->Length() > 0)
	{
		sfntly::Ptr<sfntly::LocaTable> loca_table = down_cast<sfntly::LocaTable*>(m_Fonts[vFontId].m_Font->GetTable(sfntly::Tag::loca));
		int32_t loc_length = loca_table->GlyphLength(vGlyphId);
		int32_t loc_offset = loca_table->GlyphOffset(vGlyphId);

		// Get the GLYF table for the current glyph id.
		sfntly::Ptr<sfntly::GlyphTable> glyph_table = down_cast<sfntly::GlyphTable*>(m_Fonts[vFontId].m_Font->GetTable(sfntly::Tag::glyf));
		sfntly::GlyphPtr glyph;
		glyph.Attach(glyph_table->GetGlyph(loc_offset, loc_length));

		if (glyph->GlyphType() == sfntly::GlyphType::kSimple)
		{
			auto glyphInfos = GetGlyphInfosFromGlyphId(vFontId, vGlyphId);

			if (glyphInfos)
			{
				if (glyphInfos->simpleGlyph.isValid)
				{
					MemoryStream str;
					str.WriteShort(341);
					str.WriteInt(4578);

					SimpleGlyph_Solo simpleGlyph = glyphInfos->simpleGlyph;

					auto sglyph = down_cast<sfntly::GlyphTable::SimpleGlyph*>(glyph.p_);

					int countContours = simpleGlyph.GetCountContours();
					if (countContours == 0)
					{
						simpleGlyph.LoadSimpleGlyph(sglyph);
						countContours = simpleGlyph.GetCountContours();
					}

					ct::ivec2 trans = glyphInfos->simpleGlyph.m_Translation; // first apply
					ct::dvec2 scale = glyphInfos->simpleGlyph.m_Scale; // second apply

					/////////////////////////////////////////////////////////////////////////////////////////////
					// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6glyf.html
					/////////////////////////////////////////////////////////////////////////////////////////////

					MemoryStream headerStream;
					MemoryStream flagStream;
					MemoryStream xCoordStream;
					MemoryStream yCoordStream;

					ct::iAABB boundingBox((int32_t)1e6, (int32_t)-1e6);
					int contourIdx = 0;
					ct::ivec2 last;
					for (auto &contour : simpleGlyph.coords)
					{
						int pointIdx = 0;
						for (auto &pt : contour)
						{
							if (pointIdx == 0 && contourIdx == 0)
							{
								// need to found the good translation system
								//pt.x += trans.x;
								//pt.y += trans.y;
							}

							ct::ivec2 dv = pt - last;

							int8_t flag = 0;
							if (simpleGlyph.onCurve[contourIdx][pointIdx])
								flag = flag | (1 << 0);
							flagStream.WriteByte(flag);

							// relative points
							int32_t dx = (int32_t)ct::floor(dv.x * scale.x);
							int32_t dy = (int32_t)ct::floor(dv.y * scale.y);
							xCoordStream.WriteShort(dx);
							yCoordStream.WriteShort(dy);

							// conbine absolute points
							int32_t px = (int32_t)ct::floor(pt.x * scale.x);
							int32_t py = (int32_t)ct::floor(pt.y * scale.y);
							boundingBox.Combine(ct::ivec2(px, py));

							last = pt;
							pointIdx++;
						}
						contourIdx++;
					}

					// arrange bounding box
					ct::ivec2 inf = boundingBox.lowerBound;
					ct::ivec2 sup = boundingBox.upperBound;

					inf.x = ct::mini(inf.x, 0);
					inf.y = ct::mini(inf.y, 0);

					int deltaY = 0;
					if (inf.y < glyphInfos->m_FontBoundingBox.y)
						deltaY = glyphInfos->m_FontBoundingBox.y - inf.y;
					inf.y += deltaY;
					sup.y += deltaY;

					deltaY = 0;
					if (sup.y > glyphInfos->m_FontBoundingBox.w)
						deltaY = sup.y - glyphInfos->m_FontBoundingBox.w;
					sup.y -= deltaY;

					int deltaX = 0;
					if (inf.x < glyphInfos->m_FontBoundingBox.x)
						deltaX = glyphInfos->m_FontBoundingBox.x - inf.x;
					inf.x += deltaX;
					sup.x += deltaX;

					deltaX = 0;
					if (sup.x > glyphInfos->m_FontBoundingBox.z)
						deltaX = sup.x - glyphInfos->m_FontBoundingBox.z;
					sup.x -= deltaX;

					headerStream.WriteShort(countContours);
					headerStream.WriteShort(inf.x);
					headerStream.WriteShort(inf.y);
					headerStream.WriteShort(sup.x);
					headerStream.WriteShort(sup.y);
					for (int contour = 0; contour < countContours; contour++)
						headerStream.WriteShort(sglyph->ContourEndPoint(contour));
					headerStream.WriteShort(0);

					/////////////////////////////////////////////////////////////////////////////////////////////
					/////////////////////////////////////////////////////////////////////////////////////////////

					sfntly::Ptr<sfntly::WritableFontData> finalStream;
					size_t new_lengthInBytes = headerStream.Size() + flagStream.Size() + xCoordStream.Size() + yCoordStream.Size();
					finalStream.Attach(sfntly::WritableFontData::CreateWritableFontData(new_lengthInBytes));

					int32_t offset = 0;
					finalStream->WriteBytes(offset, headerStream.Get(), 0, headerStream.Size()); offset += (int32_t)headerStream.Size();
					finalStream->WriteBytes(offset, flagStream.Get(), 0, flagStream.Size()); offset += (int32_t)flagStream.Size();
					finalStream->WriteBytes(offset, xCoordStream.Get(), 0, xCoordStream.Size()); offset += (int32_t)xCoordStream.Size();
					finalStream->WriteBytes(offset, yCoordStream.Get(), 0, yCoordStream.Size());

					/////////////////////////////////////////////////////////////////////////////////////////////
					/////////////////////////////////////////////////////////////////////////////////////////////

					//we will not do Component glyph for the moment

					return finalStream;
				}
			}
		}
	}
		
	sfntly::Ptr<sfntly::WritableFontData> writer;
	writer.Attach(sfntly::WritableFontData::CreateWritableFontData(vReadableFontData->Length()));
	vReadableFontData->CopyTo(writer);

	return writer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_assembler.cc*/
bool FontHelper::Assemble_CMap_Table()
{
	// Creating the new CMapTable and the new format 4 CMap
	sfntly::Ptr<sfntly::CMapTable::Builder> cmap_table_builder =
		down_cast<sfntly::CMapTable::Builder*>
		(m_FontBuilder->NewTableBuilder(sfntly::Tag::cmap));
	if (!cmap_table_builder)
		return false;

	sfntly::Ptr<sfntly::CMapTable::CMapFormat4::Builder> cmap_builder =
		down_cast<sfntly::CMapTable::CMapFormat4::Builder*>
		(cmap_table_builder->NewCMapBuilder(sfntly::CMapFormat::kFormat4,
			sfntly::CMapTable::WINDOWS_BMP));
	if (!cmap_builder)
		return false;

	// Creating the segments and the glyph id array
	sfntly::SegmentList* segment_list = new sfntly::SegmentList;
	sfntly::IntegerList* new_glyph_id_array = new sfntly::IntegerList;
	int32_t last_chararacter = -2;
	int32_t last_offset = 0;
	sfntly::Ptr<sfntly::CMapTable::CMapFormat4::Builder::Segment> current_segment;

	// For simplicity, we will have one segment per contiguous range.
	// To test the algorithm, we've replaced the original CMap with the CMap
	// generated by this code without removing any character.
	// Tuffy.ttf: CMap went from 3146 to 3972 bytes (1.7% to 2.17% of file)
	// AnonymousPro.ttf: CMap went from 1524 to 1900 bytes (0.96% to 1.2%)
	for (auto it = m_CharMap.begin(), e = m_CharMap.end(); it != e; ++it)
	{
		int32_t character = it->first;
		if (character != last_chararacter + 1)
		{  // new segment
			if (current_segment != NULL)
			{
				current_segment->set_end_count(last_chararacter);
				segment_list->push_back(current_segment);
			}
			// start_code = character
			// end_code = -1 (unknown for now)
			// id_delta = 0 (we don't use id_delta for this representation)
			// id_range_offset = last_offset (offset into the glyph_id_array)
			current_segment =
				new sfntly::CMapTable::CMapFormat4::Builder::
				Segment(character, -1, 0, last_offset);
		}
		int32_t old_fontid = it->second.first;
		int32_t old_glyphid = it->second.second;
		new_glyph_id_array->push_back(m_OldToNewGlyfId[FontGlyphId(old_fontid,old_glyphid)]);
		last_offset += sfntly::DataSize::kSHORT;
		last_chararacter = character;
	}

	// The last segment is still open.
	if (current_segment != NULL)
	{
		current_segment->set_end_count(last_chararacter);
		segment_list->push_back(current_segment);
	}

	// Updating the id_range_offset for every segment.
	for (int32_t i = 0, num_segs = segment_list->size(); i < num_segs; ++i)
	{
		sfntly::Ptr<sfntly::CMapTable::CMapFormat4::Builder::Segment> segment = segment_list->at(i);
		segment->set_id_range_offset(segment->id_range_offset()
			+ (num_segs - i + 1) * sfntly::DataSize::kSHORT);
	}
	
	// Adding the final, required segment.
	current_segment =
		new sfntly::CMapTable::CMapFormat4::Builder::Segment(0xffff, 0xffff, 1, 0);
	new_glyph_id_array->push_back(0);//traitement des limites
	segment_list->push_back(current_segment);
	
	// Writing the segments and glyph id array to the CMap
	cmap_builder->set_segments(segment_list);
	cmap_builder->set_glyph_id_array(new_glyph_id_array);
	delete segment_list;
	delete new_glyph_id_array;
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct LongHorMetric
{
	int32_t advanceWidth;
	int32_t lsb;
};

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/font_assembler.cc*/
bool FontHelper::Assemble_Hmtx_Hhea_Tables()
{
	auto baseFontInstance = GetBaseFontInstance();
	if (baseFontInstance)
	{
		std::vector<LongHorMetric> metrics;

		size_t fontId = 0;
		for (auto &font : m_Fonts)
		{
			if (m_NewToOldGlyfId.find(fontId) != m_NewToOldGlyfId.end())
			{
				sfntly::HorizontalMetricsTablePtr origMetrics =
					down_cast<sfntly::HorizontalMetricsTable*>(font.m_Font->GetTable(sfntly::Tag::hmtx));
				if (origMetrics == NULL)
				{
					return false;
				}

				for (size_t i = 0; i < m_NewToOldGlyfId[fontId].size(); ++i)
				{
					int32_t origGlyphId = m_NewToOldGlyfId[fontId][i];
					int32_t advanceWidth = origMetrics->AdvanceWidth(origGlyphId);
					int32_t lsb = origMetrics->LeftSideBearing(origGlyphId);

					auto glyphInfos = GetGlyphInfosFromGlyphId(fontId, origGlyphId);
					if (glyphInfos)
					{
						if (glyphInfos->simpleGlyph.isValid)
						{
							advanceWidth = (int32_t)ct::floor(advanceWidth * glyphInfos->simpleGlyph.m_Scale.x);
							lsb = (int32_t)ct::floor(lsb * glyphInfos->simpleGlyph.m_Scale.x);
						}
					}

					metrics.push_back(LongHorMetric{ advanceWidth, lsb });
				}
			}

			fontId++;
		}

		int32_t lastWidth = metrics.back().advanceWidth;
		int32_t numberOfHMetrics = (int32_t)metrics.size();
		while (numberOfHMetrics > 1 && metrics[numberOfHMetrics - 2].advanceWidth == lastWidth)
		{
			numberOfHMetrics--;
		}
		int32_t size = 4 * numberOfHMetrics + 2 * ((int32_t)metrics.size() - numberOfHMetrics);
		sfntly::WritableFontDataPtr data;
		data.Attach(sfntly::WritableFontData::CreateWritableFontData(size));
		int32_t index = 0;
		int32_t advanceWidthMax = 0;
		for (int32_t i = 0; i < numberOfHMetrics; ++i)
		{
			int32_t adw = metrics[i].advanceWidth;
			advanceWidthMax = max(adw, advanceWidthMax);
			index += data->WriteUShort(index, adw);
			index += data->WriteShort(index, metrics[i].lsb);
		}
		int32_t nMetric = (int32_t)metrics.size();
		for (int32_t j = numberOfHMetrics; j < nMetric; ++j)
		{
			index += data->WriteShort(index, metrics[j].lsb);
		}
		m_FontBuilder->NewTableBuilder(sfntly::Tag::hmtx, data);
		m_FontBuilder->NewTableBuilder(sfntly::Tag::hhea, baseFontInstance->m_Font->GetTable(sfntly::Tag::hhea)->ReadFontData());
		sfntly::HorizontalHeaderTableBuilderPtr hheaBuilder =
			down_cast<sfntly::HorizontalHeaderTable::Builder*>(m_FontBuilder->GetTableBuilder(sfntly::Tag::hhea));
		hheaBuilder->SetNumberOfHMetrics(numberOfHMetrics);
		hheaBuilder->SetAdvanceWidthMax(advanceWidthMax);

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool FontHelper::Assemble_Post_Table(std::map<CodePoint, std::string> vSelection)
{
	if (m_NewToOldGlyfId.empty() || 
		vSelection.empty())
	{
		return false;
	}

	std::vector<std::string> names;
	for (size_t fontId = 0; fontId < m_Fonts.size(); fontId++)
	{
		if (m_NewToOldGlyfId.find(fontId) != m_NewToOldGlyfId.end())
		{
			for (size_t i = 0; i < m_NewToOldGlyfId[fontId].size(); ++i)
			{
				int32_t id = m_NewToOldGlyfId[fontId][i];
				FontGlyphId fgid = FontGlyphId(fontId, id);
				if (m_ReversedCharMap.find(fgid) != m_ReversedCharMap.end()) // found
				{
					int32_t codepoint = m_ReversedCharMap[fgid];
					if (vSelection.find(codepoint) != vSelection.end()) // found
					{
						names.push_back(vSelection[codepoint]);
					}
				}
				else
				{
					names.push_back(sfntly::PostScriptTable::STANDARD_NAMES[id]);
				}
			}
		}
	}

	if (names.empty())
	{
		return true;
	}

	std::vector<int32_t> glyphNameIndices;
	sfntly::MemoryOutputStream nameBos;
	size_t nGlyphs = names.size();
	int32_t tableIndex = count_StandardNames;
	for (const auto &name : names)
	{
		int32_t glyphNameIndex;
		if (m_InvertedStandardNames.find(name) != m_InvertedStandardNames.end())
		{
			glyphNameIndex = m_InvertedStandardNames.at(name);
		}
		else
		{
			glyphNameIndex = tableIndex++;
			auto len = (int32_t)name.size();
			auto* lenptr = (uint8_t*)&len;
			nameBos.Write(lenptr, 0, sizeof(uint8_t));
			nameBos.Write((uint8_t*)name.c_str(), 0, len);
		}
		glyphNameIndices.push_back(glyphNameIndex);
	}
	std::string str = std::string((char*)nameBos.Get(), nameBos.Size());
	int32_t newLength = 2 + 2 * (int32_t)nGlyphs + (int32_t)str.size();

	// write table
	sfntly::WritableFontDataPtr header;
	header.Attach(sfntly::WritableFontData::CreateWritableFontData(size_Header + newLength));
	int32_t offset = 0;
	offset += header->WriteFixed(offset, table_Version); // version
	offset += header->WriteFixed(offset, 0); // italic
	offset += header->WriteShort(offset, 0); // underlinePosition
	offset += header->WriteShort(offset, 0); // underlineThickness
	offset += header->WriteULong(offset, 0); // isFixedPitch
	offset += header->WriteULong(offset, 0); // minMemType42
	offset += header->WriteULong(offset, 0); // maxMemType42
	offset += header->WriteULong(offset, 0); // minMemType1
	offset += header->WriteULong(offset, 0); // maxMemType1
	offset += header->WriteUShort(offset, (int32_t)nGlyphs);
	for (const auto &glyphNameIndex : glyphNameIndices)
	{
		offset += header->WriteUShort(offset, glyphNameIndex);
	}
	if (!str.empty())
	{
		offset += header->WriteBytes(offset, (uint8_t*)str.c_str(), 0, str.size());
	}

	m_FontBuilder->NewTableBuilder(sfntly::Tag::post, header);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool FontHelper::Assemble_Meta_Table()
{
	//todo: la table Meta contient les infos sur les font, comme la license , l'auteur etc..
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool FontHelper::Assemble_Head_Table()
{
	//todo: la table Meta contient les infos sur le type de font etc..
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::unordered_map<std::string, int32_t> FontHelper::InvertNameMap()
{
	std::unordered_map<std::string, int32_t> nameMap;
	for (int32_t i = 0; i < sfntly::PostScriptTable::NUM_STANDARD_NAMES; ++i)
	{
		nameMap[sfntly::PostScriptTable::STANDARD_NAMES[i]] = i;
	}
	return nameMap;
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/utils.cc*/
sfntly::Font* FontHelper::LoadFontFile(const char* font_path)
{
	sfntly::Ptr<sfntly::FontFactory> font_factory;
	font_factory.Attach(sfntly::FontFactory::GetInstance());
	sfntly::FontArray fonts;
	LoadFontFiles(font_path, font_factory, &fonts);
	return fonts[0].Detach();
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/utils.cc*/
void FontHelper::LoadFontFiles(const char* font_path, sfntly::FontFactory* factory, sfntly::FontArray* fonts)
{
	sfntly::FileInputStream input_stream;
	input_stream.Open(font_path);
	factory->LoadFonts(&input_stream, fonts);
	input_stream.Close();
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/utils.cc*/
bool FontHelper::SerializeFont(const char* font_path, sfntly::Font* font)
{
	if (!font_path)
		return false;
	sfntly::FontFactoryPtr font_factory;
	font_factory.Attach(sfntly::FontFactory::GetInstance());
	return SerializeFont(font_path, font_factory, font);
}

/* based on https://github.com/rillig/sfntly/blob/master/cpp/src/sample/subtly/utils.cc*/
bool FontHelper::SerializeFont(const char* font_path, sfntly::FontFactory* factory, sfntly::Font* font)
{
	if (!font_path || !factory || !font)
		return false;
	// Serializing the font to a stream.
	sfntly::MemoryOutputStream output_stream;
	factory->SerializeFont(font, &output_stream);
	// Serializing the stream to a file.
	FILE* output_file = NULL;
#if defined(MSVC)
	fopen_s(&output_file, font_path, "wb");
#else
	output_file = fopen(font_path, "wb");
#endif
	if (output_file == reinterpret_cast<FILE*>(NULL))
		return false;
	for (size_t i = 0; i < output_stream.Size(); ++i) {
		fwrite(&(output_stream.Get()[i]), 1, 1, output_file);
	}
	fflush(output_file);
	fclose(output_file);
	return true;
}

GlyphInfos* FontHelper::GetGlyphInfosFromGlyphId(int32_t vFontId, int32_t vGlyphId)
{
	GlyphInfos *res = 0;

	if (vFontId >= 0 && (int32_t)m_Fonts.size() > vFontId)
	{
		auto inst = &m_Fonts[vFontId];

		if (inst->m_ReversedCharMap.find(vGlyphId) != inst->m_ReversedCharMap.end()) // found
		{
			CodePoint glyphCodePoint = inst->m_ReversedCharMap[vGlyphId];

			if (inst->m_NewGlyphInfos.find(glyphCodePoint) != inst->m_NewGlyphInfos.end()) // found
			{
				res = &inst->m_NewGlyphInfos[glyphCodePoint];
			}
		}
	}

	return res;
}
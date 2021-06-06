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

#include "Generator.h"

#include <Generator/Compress.h>

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ios>
#include <sfntly/font.h>
#include <sfntly/port/file_input_stream.h>
#include <sfntly/tag.h>

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ctools/Logger.h>
#include <Generator/FontGenerator.h>
#include <Helper/Messaging.h>
#include <MainFrame.h>
#include <Panes/SourceFontPane.h>
#include <Panes/ParamsPane.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Helper/TextureHelper.h>

#include <stb/stb_image_write.h>
#include <imgui/imstb_truetype.h>

#ifdef _DEBUG
#define AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
#endif

Generator::Generator() = default;
Generator::~Generator() = default;

bool Generator::Generate(
	const std::string & vFilePath,
	const std::string & vFileName)
{
	bool res = false;

	PathStruct mainPS(ProjectFile::Instance()->m_LastGeneratedPath, ProjectFile::Instance()->m_LastGeneratedFileName, "");

	if (!vFilePath.empty()) mainPS.path = vFilePath;
	if (!vFileName.empty()) mainPS.name = vFileName;

	if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_CURRENT) &&
		ProjectFile::Instance()->m_SelectedFont.use_count())
	{
		if (ProjectFile::Instance()->m_SelectedFont->IsGenMode(GENERATOR_MODE_SRC))
		{
			res = GenerateSource_One(
				mainPS.GetFPNE(),
				ProjectFile::Instance()->m_SelectedFont,
				ProjectFile::Instance()->m_SelectedFont->m_GenModeFlags);
		}
		else if (ProjectFile::Instance()->m_SelectedFont->IsGenMode(GENERATOR_MODE_FONT))
		{
			res = GenerateFontFile_One(
				mainPS.GetFPNE(),
				ProjectFile::Instance()->m_SelectedFont,
				ProjectFile::Instance()->m_SelectedFont->m_GenModeFlags);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
			if (res)
				ParamsPane::Instance()->OpenFont(mainPS.GetFPNE(), false); // directly load the generated font file
#endif
		}
		else if (ProjectFile::Instance()->m_SelectedFont->IsGenMode(GENERATOR_MODE_CARD))
		{
			res = GenerateCard_One(
				mainPS.GetFPNE_WithExt("png"),
				ProjectFile::Instance()->m_SelectedFont);
		}
	}
	else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_BATCH))
	{
		for (auto font : ProjectFile::Instance()->m_Fonts)
		{
			if (font.second.use_count() &&
				font.second->m_EnabledForGeneration) // actif dans le per font pour la generation
			{
				std::string fileName = font.second->m_FontFileName;
				if (!font.second->m_GeneratedFileName.empty())
					fileName = font.second->m_GeneratedFileName;
				auto ps = FileHelper::Instance()->ParsePathFileName(fileName);
				if (ps.isOk)
				{
					// settings per font
					if (font.second->IsGenMode(GENERATOR_MODE_SRC))
					{
						GenerateSource_One(
							ps.GetFPNE_WithPath(mainPS.path),
							font.second,
							font.second->m_GenModeFlags);
					}
					else if (font.second->IsGenMode(GENERATOR_MODE_FONT))
					{
						res = GenerateFontFile_One(
							ps.GetFPNE_WithPath(mainPS.path),
							font.second,
							font.second->m_GenModeFlags);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
						if (res)
							ParamsPane::Instance()->OpenFont(mainPS.GetFPNE(), false); // directly load the generated font file
#endif
					}
					else if (font.second->IsGenMode(GENERATOR_MODE_CARD))
					{
						std::string fileName = font.second->m_FontFileName;
						if (!font.second->m_GeneratedFileName.empty())
							fileName = font.second->m_GeneratedFileName;
						auto ps = FileHelper::Instance()->ParsePathFileName(fileName);
						if (ps.isOk)
						{
							res = GenerateCard_One(
								ps.GetFPNE_WithPathExt(mainPS.path, "png"),
								font.second);
						}
					}
				}
			}
		}
	}
	else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_MERGED))
	{
		if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_SRC))
		{
			res = GenerateSource_Merged(
				mainPS.GetFPNE(),
				ProjectFile::Instance()->m_GenModeFlags);
		}
		else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_FONT))
		{
			res = GenerateFontFile_Merged(
				mainPS.GetFPNE(),
				ProjectFile::Instance()->m_GenModeFlags);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
			if (res)
				ParamsPane::Instance()->OpenFont(mainPS.GetFPNE(), false); // directly load the generated font file
#endif
		}
		else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_CARD))
		{
			res = GenerateCard_Merged(mainPS.GetFPNE_WithExt("png"));
		}
	}


	return res;
}

///////////////////////////////////////////////////////////////////////////////////
//// CARD GENERATION //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool Generator::WriteGlyphCardToPicture(
	const std::string & vFilePathName,
	std::map<std::string, std::pair<uint32_t, size_t>> vLabels, // lable, codepoint, FontInfos ptr
	const uint32_t & vGlyphHeight, const uint32_t & vMaxRows)
{
	bool res = false;

	if (vGlyphHeight && vMaxRows)
	{
		stbtt_fontinfo labelFontInfo;

		// FontInfos ptr (size_t), stbtt_fontinfo // size_t is alwasy the size of the address (uint32_t for x32, uint64_t for x64)
		std::unordered_map<size_t, stbtt_fontinfo> fonts;

		auto io = &ImGui::GetIO();
		if (!io->Fonts->ConfigData.empty())
		{
			const int32_t font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)io->Fonts->ConfigData[0].FontData,
				io->Fonts->ConfigData[0].FontNo);
			if (stbtt_InitFont(&labelFontInfo, (unsigned char*)io->Fonts->ConfigData[0].FontData, font_offset))
			{
				// will write one glyph labeled

				uint32_t labelHeight = (uint32_t)(vGlyphHeight * 0.5f);
				uint32_t glyphHeight = (uint32_t)(vGlyphHeight * 0.8f);
				uint32_t padding_x = (uint32_t)(vGlyphHeight * 0.1f);
				uint32_t padding_y = 2U;
				uint32_t columnCount = (uint32_t)ceil((double)vLabels.size() / (double)vMaxRows);

				// max width of the labels
				uint32_t maxLabelWidth = 0U;
				for (const auto& it : vLabels)
				{
					maxLabelWidth = ct::maxi(maxLabelWidth, (uint32_t)it.first.size());
				}

				maxLabelWidth *= labelHeight; // one mult instead of many in loops for same result

				// extend max width of the buffer for column count
				uint32_t maxWidthOfOneItem = glyphHeight + maxLabelWidth;

				// array of bytes
				std::vector<uint8_t> buffer;
				uint32_t bufferWidth = maxWidthOfOneItem * columnCount;
				uint32_t bufferHeight = vGlyphHeight * (vMaxRows + 2U);
				buffer.resize((size_t)bufferWidth * (size_t)bufferHeight);
				memset(buffer.data(), 0, buffer.size());

				// iteration pof labels
				int32_t xpos = padding_x;
				int32_t ypos = padding_y;
				int32_t CurColumnOffset = 0;

				// we will compute final size accoridng to the glyph and labels
				int32_t finalWidth = 0;
				int32_t finalHeight = 0;
				int32_t columnMaxWidth = 0;

				int32_t countRows = 0;
				for (const auto& it : vLabels)
				{
					uint32_t codePoint = it.second.first;
					auto fontPtr = (FontInfos*)it.second.second;
					if (fonts.find((size_t)fontPtr) == fonts.end())
					{
						stbtt_fontinfo fontInfos;
						// not exist so we will load the stbtt_fontinfo
						if (!fontPtr->m_ImFontAtlas.ConfigData.empty())
						{
							const int32_t font_offset2 = stbtt_GetFontOffsetForIndex(
								(unsigned char*)fontPtr->m_ImFontAtlas.ConfigData[0].FontData,
								fontPtr->m_ImFontAtlas.ConfigData[0].FontNo);
							if (stbtt_InitFont(&fontInfos, (unsigned char*)fontPtr->m_ImFontAtlas.ConfigData[0].FontData, font_offset2))
							{
								fonts[(size_t)fontPtr] = fontInfos;
							}
						}
					}

					if (fonts.find((size_t)fontPtr) != fonts.end())
					{
						const auto& glyphFontInfos = fonts[(size_t)fontPtr];

						std::string lblToRender = " " + it.first;
						auto text = lblToRender.c_str();

						xpos = CurColumnOffset + padding_x;

						// one char for the glyph
						float glyphScale = stbtt_ScaleForPixelHeight(&glyphFontInfos, (float)(glyphHeight));
						int32_t ascent, descent, baseline;
						stbtt_GetFontVMetrics(&glyphFontInfos, &ascent, &descent, 0);
						baseline = (int32_t)(ascent * glyphScale);
						int32_t advance, lsb, x0, y0, x1, y1;
						stbtt_GetCodepointHMetrics(&glyphFontInfos, codePoint, &advance, &lsb);
						float x_shift = vGlyphHeight * 0.5f - (advance * glyphScale) * 0.5f;
						float y_shift = vGlyphHeight * 0.5f - (ascent - descent) * glyphScale * 0.5f;
						stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfos, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
						int32_t x = (uint32_t)xpos + x0;
						int32_t y = baseline + y0;
						while (x < 0 || y < 0) // we decrease scale until glyph can be added in picture
						{
							glyphScale *= 0.9f;
							x_shift = vGlyphHeight * 0.5f - (advance * glyphScale) * 0.5f;
							y_shift = vGlyphHeight * 0.5f - (ascent - descent) * glyphScale * 0.5f;
							stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfos, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
							x = (uint32_t)xpos + x0;
							y = baseline + y0;
						}
						if (x >= 0 && y >= 0)
						{
							uint8_t* ptr = buffer.data() + (size_t)(bufferWidth * ((size_t)ypos + (size_t)y) + (size_t)x);
							stbtt_MakeCodepointBitmapSubpixel(&glyphFontInfos, ptr, x1 - x0, y1 - y0, bufferWidth, glyphScale, glyphScale, 0, 0, codePoint);
						}
						xpos += vGlyphHeight;

						// the rest for the label
						float labelScale = stbtt_ScaleForPixelHeight(&labelFontInfo, (float)(labelHeight));
						stbtt_GetFontVMetrics(&labelFontInfo, &ascent, &descent, 0);
						baseline = (int32_t)(ascent * labelScale);

						int32_t ch = 0;
						while (text[ch])
						{
							stbtt_GetCodepointHMetrics(&labelFontInfo, text[ch], &advance, &lsb);
							y_shift = vGlyphHeight * 0.5f - labelHeight * 0.5f;
							stbtt_GetCodepointBitmapBoxSubpixel(&labelFontInfo, text[ch], labelScale, labelScale, 0, y_shift, &x0, &y0, &x1, &y1);

							x = (uint32_t)xpos + x0;
							y = baseline + y0;
							float newLabelScale = labelScale;
							while (x < 0 || y < 0) // we decrease scale until glyph can be added in picture
							{
								newLabelScale *= 0.9f;
								stbtt_GetCodepointBitmapBoxSubpixel(&labelFontInfo, codePoint, newLabelScale, newLabelScale, 0, y_shift, &x0, &y0, &x1, &y1);
								x = (uint32_t)xpos + x0;
								y = baseline + y0;
							}
							if (x >= 0 && y >= 0)
							{
								uint8_t* ptr = buffer.data() + (size_t)(bufferWidth * ((size_t)ypos + (size_t)y) + (size_t)x);
								stbtt_MakeCodepointBitmapSubpixel(&labelFontInfo, ptr, x1 - x0, y1 - y0, bufferWidth, newLabelScale, newLabelScale, 0, 0, text[ch]);
							}

							xpos += (int32_t)(advance * newLabelScale);
							if (text[ch + 1])
								xpos += (int32_t)(labelScale * stbtt_GetCodepointKernAdvance(&labelFontInfo, text[ch], text[ch + 1]));
							++ch;
						}

						// inc of the row count
						countRows++;

						// extra space
						xpos += (int32_t)(advance * labelScale);
						ypos += vGlyphHeight;

						// max width of the current column
						columnMaxWidth = ct::maxi(columnMaxWidth, xpos - CurColumnOffset);

						// we compute final size according to the glyph and labels wrotes in buffer
						finalWidth = ct::maxi(finalWidth, xpos);
						finalHeight = ct::maxi(finalHeight, ypos);

						// column change if needed
						if (countRows % vMaxRows == 0) // we need to change the column
						{
							ypos = padding_y;
							CurColumnOffset += columnMaxWidth + padding_x;
							columnMaxWidth = 0;
						}
					}

				}

				if (finalWidth && finalHeight)
				{
					int32_t success = stbi_write_png(
						vFilePathName.c_str(),
						finalWidth + padding_x,
						finalHeight,
						1,
						buffer.data(),
						bufferWidth);

					if (success)
					{
						res = true;
					}
					else
					{
						Messaging::Instance()->AddError(true, nullptr, nullptr,
							"Png Writing Fail for path : %s", vFilePathName.c_str());
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, nullptr, nullptr,
						"Png Writing Fail for path : %s, final computed size not ok %i, %i\n",
						vFilePathName.c_str(), finalWidth, finalHeight);
				}
			}
		}
	}

	return res;
}

static std::string GetNewHeaderName(const std::string & vPrefix, const std::string & vName)
{
	std::string glyphName = vName;
	ct::replaceString(glyphName, " ", "_");
	ct::replaceString(glyphName, "-", "_");

	// by ex .notdef will become DOT_notdef
	// because a define with '.' is a problem for the compiler
	ct::replaceString(glyphName, ".", "DOT_");

	// UpperCase
	for (auto& c : glyphName)
		c = (char)toupper((int32_t)c);

	return vPrefix + "_" + glyphName;
}

/*
Generate Card pictureHeader file with glyphs and labels
two modes :
- if not glyph were selected => will generate header for all font glyphs
- if glyph were selected => will generate header only for these glyphs
*/
bool Generator::GenerateCard_One(
	const std::string & vFilePathName,
	std::shared_ptr<FontInfos> vFontInfos)
{
	bool res = false;

	if (!vFilePathName.empty() && vFontInfos.use_count())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			// file
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, "png");

			// Prefix
			std::string prefix = "";
			if (!vFontInfos->m_FontPrefix.empty())
				prefix = "ICON_" + vFontInfos->m_FontPrefix;

			// generate bd of old codepoint and new string
			// old codepoint because, we will get the glyph from te already loaded texture,
			// so can only be accessed with old codepoints
			std::map<std::string, std::pair<uint32_t, size_t>> glyphs;
			if (vFontInfos->m_SelectedGlyphs.empty()) // no glyph selected so generate for whole font
			{
				for (auto& glyph : vFontInfos->m_GlyphCodePointToName)
				{
					glyphs[GetNewHeaderName(prefix, glyph.second)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)vFontInfos.get());
				}
			}
			else
			{
				for (auto& glyph : vFontInfos->m_SelectedGlyphs)
				{
					if (glyph.second)
					{
						glyphs[GetNewHeaderName(prefix, glyph.second->newHeaderName)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)vFontInfos.get());
					}
				}
			}

			res = WriteGlyphCardToPicture(filePathName, glyphs, vFontInfos->m_CardGlyphHeightInPixel, vFontInfos->m_CardCountRowsMax);
			if (res && ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO))
				FileHelper::Instance()->OpenFile(vFilePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr, "Invalid path : %s", vFilePathName.c_str());
		}
	}

	return res;
}

bool Generator::GenerateCard_Merged(
	const std::string & vFilePathName)
{
	bool res = false;

	UNUSED(vFilePathName);


	if (!vFilePathName.empty() &&
		!ProjectFile::Instance()->m_Fonts.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			// file
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, "png");

			// Prefix
			std::string prefix = "";
			if (!ProjectFile::Instance()->m_MergedFontPrefix.empty())
				prefix = "ICON_" + ProjectFile::Instance()->m_MergedFontPrefix;

			// generate bd of old codepoint and new string
			// old codepoint because, we will get the glyph from te already loaded texture,
			// so can only be accessed with old codepoints
			std::map<std::string, std::pair<uint32_t, size_t>> glyphs;
			for (const auto font : ProjectFile::Instance()->m_Fonts)
			{
				if (font.second)
				{
					for (const auto& glyph : font.second->m_SelectedGlyphs)
					{
						if (glyph.second)
						{
							glyphs[GetNewHeaderName(prefix, glyph.second->newHeaderName)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)&font.second);
						}
					}
				}
			}

			res = WriteGlyphCardToPicture(filePathName, glyphs, ProjectFile::Instance()->m_MergedCardGlyphHeightInPixel, ProjectFile::Instance()->m_MergedCardCountRowsMax);
			if (res && ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO))
				FileHelper::Instance()->OpenFile(vFilePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr, "Invalid path : %s", vFilePathName.c_str());
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////
//// FONT GENERATION //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* 03/03/2020 22h41 it work like a charm (Yihaaaa!!) */
/*
Generate Font File with selected Glyphs
if vGenerateHeader is true, will Generate also a Header file with glyphs
   code like in https://github.com/juliettef/IconFontCppHeaders
*/
bool Generator::GenerateFontFile_One(
	const std::string & vFilePathName,
	std::shared_ptr<FontInfos> vFontInfos,
	const GenModeFlags & vFlags)
{
	bool res = false;

	if (!vFilePathName.empty() && vFontInfos.use_count())
	{
		FontGenerator fontGenerator;

		std::map<int32_t, std::string> newHeaderNames;
		std::map<int32_t, int32_t> newCodePoints;
		std::map<CodePoint, std::shared_ptr<GlyphInfos>> newGlyphInfos;
		if (!vFontInfos->m_SelectedGlyphs.empty())
		{
			for (const auto& glyph : vFontInfos->m_SelectedGlyphs)
			{
				if (glyph.second)
				{
					newHeaderNames[glyph.first] = glyph.second->newHeaderName;
					newCodePoints[glyph.first] = glyph.second->newCodePoint;
					newGlyphInfos[glyph.first] = glyph.second;
				}
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"No glyphs are seleted for font file %s. aborting.\n",
				ProjectFile::Instance()->m_SelectedFont->m_FontFilePathName.c_str());
			return false;
		}

		if (!newHeaderNames.empty() && !newCodePoints.empty())
		{
			std::string absPath = ProjectFile::Instance()->GetAbsolutePath(vFontInfos->m_FontFilePathName);
			if (!fontGenerator.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, true))
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr,
					"Could not open font file %s.\n",
					ProjectFile::Instance()->m_SelectedFont->m_FontFilePathName.c_str());
				return false;
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"No glyphs header or codepoint found for font file %s. aborting.\n",
				ProjectFile::Instance()->m_SelectedFont->m_FontFilePathName.c_str());
			return false;
		}

		/*	std::set<int32_t> table_blacklist;
			table_blacklist.insert(sfntly::Tag::DSIG);
			table_blacklist.insert(sfntly::Tag::GDEF);
			table_blacklist.insert(sfntly::Tag::GPOS);
			table_blacklist.insert(sfntly::Tag::GSUB);
			table_blacklist.insert(sfntly::Tag::kern);
			table_blacklist.insert(sfntly::Tag::hdmx);
			table_blacklist.insert(sfntly::Tag::vmtx);
			table_blacklist.insert(sfntly::Tag::VDMX);
			table_blacklist.insert(sfntly::Tag::LTSH);
			table_blacklist.insert(sfntly::Tag::vhea);
			table_blacklist.insert(sfntly::Tag::morx);
			not used for the moment, we keep only head table
		*/

		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_CURRENT))
			{
				vFontInfos->m_GeneratedFileName = ps.name;
			}

			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, "ttf");

			if (fontGenerator.GenerateFontFile(filePathName, vFlags & GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
			{
				res = true;

				if (vFlags & GENERATOR_MODE_HEADER)
				{
					m_HeaderGenerator.GenerateHeader_One(
						filePathName,
						vFontInfos);
				}
				if (vFlags & GENERATOR_MODE_CARD)
				{
					GenerateCard_One(
						filePathName,
						vFontInfos);
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr, "Cannot create font file %s", filePathName.c_str());
				return false;
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr, "File Path Name is wrong : %s", vFilePathName.c_str());
			return false;
		}
	}

	return res;
}

bool Generator::GenerateFontFile_Merged(
	const std::string & vFilePathName,
	const GenModeFlags & vFlags)
{
	bool res = false;

	if (ProjectFile::Instance()->IsLoaded() &&
		!vFilePathName.empty() &&
		!ProjectFile::Instance()->m_Fonts.empty() &&
		!ProjectFile::Instance()->m_FontToMergeIn.empty())
	{
		FontGenerator fontGenerator;

		bool tasks = true;

		ct::ivec2 baseSize = 0;
		ct::ivec4 baseFontBoundingBox;
		int32_t baseFontAscent = 0;
		int32_t baseFontDescent = 0;

		// abse infos for merge all toher fonts in this one
		for (auto it : ProjectFile::Instance()->m_Fonts)
		{
			if (it.second)
			{
				if (ProjectFile::Instance()->m_FontToMergeIn == it.second->m_FontFileName &&
					!ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
				{
					baseFontBoundingBox = it.second->m_BoundingBox;
					baseFontAscent = it.second->m_Ascent;
					baseFontDescent = it.second->m_Descent;
					baseSize = it.second->m_BoundingBox.zw() - it.second->m_BoundingBox.xy();
				}
			}
		}

		for (auto it : ProjectFile::Instance()->m_Fonts)
		{
			if (it.second)
			{
				bool scaleChanged = false;
				ct::dvec2 scale = 1.0;
				if (ProjectFile::Instance()->m_FontToMergeIn != it.second->m_FontFileName &&
					!ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
				{
					scaleChanged = true;
					ct::ivec2 newSize = it.second->m_BoundingBox.zw() - it.second->m_BoundingBox.xy();
					scale.x = (double)baseSize.x / (double)newSize.x;
					scale.y = (double)baseSize.y / (double)newSize.y;
					double v = ct::mini(scale.x, scale.y);
					scale.x = v; // same value for keep glyph ratio
					scale.y = v; // same value for keep glyph ratio
				}

				std::map<int32_t, std::string> newHeaderNames;
				std::map<int32_t, int32_t> newCodePoints;
				std::map<CodePoint, std::shared_ptr<GlyphInfos>> newGlyphInfos;
				for (const auto& glyph : it.second->m_SelectedGlyphs)
				{
					if (glyph.second)
					{
						newHeaderNames[glyph.first] = glyph.second->newHeaderName;
						newCodePoints[glyph.first] = glyph.second->newCodePoint;

						if (scaleChanged &&
							!ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
						{
							glyph.second->simpleGlyph.isValid = true;
							glyph.second->simpleGlyph.m_Scale = ImVec2((float)scale.x, (float)scale.y);

							glyph.second->m_FontBoundingBox = baseFontBoundingBox;
							glyph.second->m_FontAscent = baseFontAscent;
							glyph.second->m_FontDescent = baseFontDescent;
						}

						newGlyphInfos[glyph.first] = glyph.second;
					}
				}

				if (!newHeaderNames.empty())
				{
					std::string absPath = ProjectFile::Instance()->GetAbsolutePath(it.second->m_FontFilePathName);
					tasks &= fontGenerator.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, !scaleChanged);
				}
				else
				{
					tasks = false;
				}
			}
		}

		if (tasks)
		{
			std::set<int32_t> table_blacklist;
			/*
				table_blacklist.insert(sfntly::Tag::DSIG);
				table_blacklist.insert(sfntly::Tag::GDEF);
				table_blacklist.insert(sfntly::Tag::GPOS);
				table_blacklist.insert(sfntly::Tag::GSUB);
				table_blacklist.insert(sfntly::Tag::kern);
				table_blacklist.insert(sfntly::Tag::hdmx);
				table_blacklist.insert(sfntly::Tag::vmtx);
				table_blacklist.insert(sfntly::Tag::VDMX);
				table_blacklist.insert(sfntly::Tag::LTSH);
				table_blacklist.insert(sfntly::Tag::vhea);
				table_blacklist.insert(sfntly::Tag::morx);
				not used for the moment, we keep only head table
			*/

			std::string filePathName = vFilePathName;
			auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
			if (ps.isOk)
			{
				std::string name = ps.name;
				ct::replaceString(name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt(name, "ttf");

				if (fontGenerator.GenerateFontFile(vFilePathName, vFlags & GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
				{
					res = true;

					if (vFlags & GENERATOR_MODE_HEADER)
					{
						m_HeaderGenerator.GenerateHeader_Merged(
							filePathName);
					}
					if (vFlags & GENERATOR_MODE_CARD)
					{
						GenerateCard_Merged(
							filePathName);
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, nullptr, nullptr, "Cannot create font file %s", filePathName.c_str());
					return res;
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr, "File Path Name is wrong : %s", vFilePathName.c_str());
				return res;
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"Could not open font file %s.\n",
				ProjectFile::Instance()->m_SelectedFont->m_FontFilePathName.c_str());
			return res;
		}
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////
//// CPP GENERATION ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* 03/03/2020 23h38 it work like a charm (Wouhoooooo!!)
will generate cpp fille with/without header
two modes :
- no glyph selected => export whole font file to cpp (and header is asked)
- some glyohs selectef => export glyph selection in a new temporary font file, then exported in cpp (and header if asked)
*/
bool Generator::GenerateSource_One(
	const std::string & vFilePathName,
	std::shared_ptr<FontInfos> vFontInfos,
	const GenModeFlags & vFlags)
{
	bool res = false;
	if (!vFilePathName.empty() && vFontInfos.use_count())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string buffer;

			bool generateTemporaryFontFile = false;
			if (vFontInfos->m_SelectedGlyphs.empty()) // export whole font file to cpp (and header is asked)
			{
				filePathName = ProjectFile::Instance()->GetAbsolutePath(vFontInfos->m_FontFilePathName);
				// todo: il faut gerer le cas ou on va exporter tout les fichier sans selction
				// ici on ne fait que l'actif, c'est aps ce qu'on veut
			}
			else // export glyph selection in a new temporary font file, then exported in cpp (and header if asked)
			{
				// generate temporary font file first
				generateTemporaryFontFile = true;

				std::string name = ps.name;
				ct::replaceString(name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt("temporary_" + name, "ttf");

				GenerateFontFile_One(filePathName, vFontInfos,
					(GenModeFlags)(vFlags & ~GENERATOR_MODE_HEADER_CARD)); // no header or card to generate
			}

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_CURRENT))
				{
					vFontInfos->m_GeneratedFileName = ps.name;
				}

				std::string lang;
				if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_C)) lang = "c";
				else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CPP)) lang = "cpp";
				else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CSHARP)) lang = "c#";
				if (!lang.empty())
				{
					std::string bufferName;
					size_t bufferSize = 0;
					buffer = Compress::GetCompressedBase85BytesArray(
						lang,
						filePathName,
						vFontInfos->m_FontPrefix,
						&bufferName,
						&bufferSize);

					if (generateTemporaryFontFile)
					{
						// we have the result, if empty or not we need to destroy the temporary font file
						FileHelper::Instance()->DestroyFile(filePathName);
					}

					// if ok, serialization
					if (!buffer.empty() && !bufferName.empty() && bufferSize > 0)
					{
						PathStruct psSource = ps;

						std::string sourceExt;
						if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_C)) sourceExt = "c";
						else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CPP)) sourceExt = "cpp";
						else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
						{
							psSource.name += "_Bytes";
							sourceExt = "cs";
						}

						std::string sourceFile;

						if (vFlags & GENERATOR_MODE_HEADER)
						{
							PathStruct psHeader = ps;

							std::string headerExt;
							if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_C) ||
								vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CPP)) headerExt = "h";
							else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
							{
								psHeader.name += "_Labels";
								headerExt = "cs";
							}

							if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_C) ||
								vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CPP))
							{
								sourceFile = "#include \"" + ps.name + "." + headerExt + "\"\n\n";
							}

							m_HeaderGenerator.GenerateHeader_One(
								psHeader.GetFPNE_WithExt(headerExt),
								vFontInfos,
								bufferName,
								bufferSize);
						}

						if (vFlags & GENERATOR_MODE_CARD)
						{
							GenerateCard_One(
								ps.GetFPNE_WithExt("png"),
								vFontInfos);
						}

						if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
						{
							sourceFile += "using System;\n";
							sourceFile += "using System.Collections.Generic;\n\n";
							sourceFile += ct::toStr("namespace IconFonts\n{\n\tpublic static class %s_Bytes\n\t{ \n", ProjectFile::Instance()->m_MergedFontPrefix.c_str());
							sourceFile += buffer;
							sourceFile += "\t}\n}\n";
						}
						else if (vFontInfos->IsGenMode(GENERATOR_MODE_LANG_C) ||
							vFontInfos->IsGenMode(GENERATOR_MODE_LANG_CPP))
						{
							sourceFile += buffer;
						}

						filePathName = psSource.GetFPNE_WithExt(sourceExt);
						FileHelper::Instance()->SaveStringToFile(sourceFile, filePathName);
						if (vFlags & GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO)
							FileHelper::Instance()->OpenFile(filePathName);

						res = true;
					}
					else
					{
						Messaging::Instance()->AddError(true, nullptr, nullptr,
							"Error opening or reading file %s", filePathName.c_str());
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, nullptr, nullptr,
						"Language not set for : %s", vFilePathName.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr,
					"Cant open file %s", filePathName.c_str());
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"Bad File path Name %s", filePathName.c_str());
		}
	}

	return res;
}

bool Generator::GenerateSource_Merged(
	const std::string & vFilePathName,
	const GenModeFlags & vFlags)
{
	bool res = false;

	if (!vFilePathName.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string buffer;

			ct::replaceString(ps.name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt("temporary_" + ps.name, "ttf");

			GenerateFontFile_Merged(
				filePathName,
				(GenModeFlags)(vFlags & ~GENERATOR_MODE_HEADER_CARD)); // no header to generate

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				std::string lang;
				if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_C)) lang = "c";
				else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CPP)) lang = "cpp";
				else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CSHARP)) lang = "c#";
				if (!lang.empty())
				{
					std::string bufferName;
					size_t bufferSize = 0;
					buffer = Compress::GetCompressedBase85BytesArray(
						lang,
						filePathName,
						ProjectFile::Instance()->m_MergedFontPrefix,
						&bufferName,
						&bufferSize);

					// we have the result, if empty or not we need to destroy the temporary font file
					FileHelper::Instance()->DestroyFile(filePathName);

					// if ok, serialization
					if (!buffer.empty() && !bufferName.empty() && bufferSize > 0)
					{
						PathStruct psSource = ps;

						std::string sourceExt;
						if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_C)) sourceExt = "c";
						else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CPP)) sourceExt = "cpp";
						else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
						{
							psSource.name += "_Bytes";
							sourceExt = "cs";
						}

						std::string sourceFile;

						if (vFlags & GENERATOR_MODE_HEADER)
						{
							PathStruct psHeader = ps;

							std::string headerExt;
							if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_C) ||
								ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CPP)) headerExt = "h";
							else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
							{
								psHeader.name += "_Labels";
								headerExt = "cs";
							}

							if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_C) ||
								ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CPP))
							{
								sourceFile = "#include \"" + ps.name + "." + headerExt + "\"\n\n";
								std::string prefix = "";
								prefix = "FONT_ICON_BUFFER_NAME_" + ProjectFile::Instance()->m_MergedFontPrefix;
								ct::replaceString(buffer, ProjectFile::Instance()->m_MergedFontPrefix + "_compressed_data_base85", prefix);
							}

							m_HeaderGenerator.GenerateHeader_Merged(
								psHeader.GetFPNE_WithExt(headerExt),
								bufferName,
								bufferSize);
						}

						if (vFlags & GENERATOR_MODE_CARD)
						{
							GenerateCard_Merged(
								ps.GetFPNE_WithExt("png"));
						}

						if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
						{
							sourceFile += "using System;\n";
							sourceFile += "using System.Collections.Generic;\n\n";
							sourceFile += ct::toStr("namespace IconFonts\n{\n\tpublic static class %s_Bytes\n\t{ \n", ProjectFile::Instance()->m_MergedFontPrefix.c_str());
							sourceFile += buffer;
							sourceFile += "\t}\n}\n";
						}
						else if (ProjectFile::Instance()->IsGenMode(GENERATOR_MODE_LANG_CPP))
						{
							sourceFile += buffer;
						}

						filePathName = psSource.GetFPNE_WithExt(sourceExt);
						FileHelper::Instance()->SaveStringToFile(sourceFile, filePathName);
						if (vFlags & GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO)
							FileHelper::Instance()->OpenFile(filePathName);
						res = true;
					}
					else
					{
						Messaging::Instance()->AddError(true, nullptr, nullptr,
							"Error opening or reading file %s", filePathName.c_str());
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, nullptr, nullptr,
						"Language not set for : %s", vFilePathName.c_str());
				}
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"Bad File path Name %s", filePathName.c_str());
		}
	}

	return res;
}

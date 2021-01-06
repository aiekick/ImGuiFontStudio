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
#include <Helper/FontHelper.h>
#include <Helper/Messaging.h>
#include <MainFrame.h>
#include <Panes/SourceFontPane.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <imgui/imstb_truetype.h>

//#define AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE

Generator::Generator() = default;
Generator::~Generator() = default;

void Generator::Generate(
	ProjectFile *vProjectFile,
	const std::string& vFilePath,
	const std::string& vFileName)
{
	if (vProjectFile)
	{
		PathStruct mainPS(vProjectFile->m_LastGeneratedPath, vProjectFile->m_LastGeneratedFileName, "");
		
		if (!vFilePath.empty()) mainPS.path = vFilePath;
		if (!vFileName.empty()) mainPS.name = vFileName;

		if (vProjectFile->IsGenMode(GENERATOR_MODE_CPP))
		{
			if (vProjectFile->IsGenMode(GENERATOR_MODE_CURRENT))
			{
				GenerateCpp_One(
					mainPS.GetFPNE(),
					vProjectFile, 
					vProjectFile->m_SelectedFont, 
					vProjectFile->m_GenMode);
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_BATCH))
			{
				for (auto &font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						GenerateCpp_One(
							ps.GetFPNE_WithPath(vFilePath),
							vProjectFile, 
							&font.second, 
							vProjectFile->m_GenMode);
					}
				}
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED))
			{
				GenerateCpp_Merged(
					mainPS.GetFPNE(),
					vProjectFile, 
					vProjectFile->GetGenMode());
			}
		}
		else if (vProjectFile->IsGenMode(GENERATOR_MODE_FONT))
		{
			if (vProjectFile->IsGenMode(GENERATOR_MODE_CURRENT))
			{
				GenerateFontFile_One(
					mainPS.GetFPNE(),
					vProjectFile, 
					vProjectFile->m_SelectedFont, 
					vProjectFile->m_GenMode);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
				SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_BATCH))
			{
				for (auto &font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						GenerateFontFile_One(
							ps.GetFPNE_WithPath(vFilePath),
							vProjectFile, 
							&font.second, 
							vProjectFile->m_GenMode);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
						SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
					}
				}
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED))
			{
				GenerateFontFile_Merged(
					mainPS.GetFPNE(),
					vProjectFile,
					vProjectFile->m_GenMode);
#ifdef AUTO_OPEN_FONT_IN_APP_AFTER_GENERATION_FOR_DEBUG_PURPOSE
				SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
			}
		}
		else if (vProjectFile->IsGenMode(GENERATOR_MODE_CARD))
		{
			if (vProjectFile->IsGenMode(GENERATOR_MODE_CURRENT))
			{
				GenerateCard_One(
					mainPS.GetFPNE_WithExt(".png"),
					vProjectFile->m_SelectedFont,
					vProjectFile->m_CardGlyphHeightInPixel,
					vProjectFile->m_CardCountRowsMax);
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_BATCH))
			{
				for (auto& font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						GenerateCard_One(
							ps.GetFPNE_WithPathExt(vFilePath, ".png"),
							&font.second,
							vProjectFile->m_CardGlyphHeightInPixel,
							vProjectFile->m_CardCountRowsMax);
					}
				}
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_MERGED))
			{
				GenerateCard_Merged(
					mainPS.GetFPNE_WithExt(".png"),
					vProjectFile,
					vProjectFile->m_CardGlyphHeightInPixel,
					vProjectFile->m_CardCountRowsMax);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// STATIC TEXTURE TO PICTURE FILE ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool Generator::SaveTextureToPng(GLFWwindow* vWin, const std::string& vFilePathName, 
	GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount)
{
	bool res = false;

	if (!vFilePathName.empty() && vWin)
	{
		glfwMakeContextCurrent(vWin);

		std::vector<uint8_t> bytes;

		bytes.resize((size_t)vTextureSize.x * (size_t)vTextureSize.y * (size_t)vChannelCount); // 1 channel only

		GLenum format = GL_RGBA;
		if (vChannelCount == 1) format = GL_RED;
		if (vChannelCount == 2) format = GL_RG;
		if (vChannelCount == 3) format = GL_RGB;
		if (vChannelCount == 4) format = GL_RGBA;

		// Upload texture to graphics system
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glBindTexture(GL_TEXTURE_2D, vTextureId);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, bytes.data());
		glBindTexture(GL_TEXTURE_2D, last_texture);

		int32_t resWrite = stbi_write_png(
			vFilePathName.c_str(),
			vTextureSize.x,
			vTextureSize.y,
			vChannelCount,
			bytes.data(),
			vTextureSize.x * vChannelCount);

		res = (resWrite > 0);
	}

	return res;
}

///////////////////////////////////////////////////////////////////////////////////
//// CARD GENERATION //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static void WriteGlyphCardToPicture(
	const std::string& vFilePathName, 
	std::map<std::string, std::pair<uint32_t, size_t>> vLabels, // lable, codepoint, FontInfos ptr
	const uint32_t& vGlyphHeight, const uint32_t& vMaxRows)
{
	if (vGlyphHeight && vMaxRows)
	{
		stbtt_fontinfo labelFontInfo;

		bool labelFontLoaded = false;

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
							const int32_t font_offset = stbtt_GetFontOffsetForIndex(
								(unsigned char*)fontPtr->m_ImFontAtlas.ConfigData[0].FontData,
								fontPtr->m_ImFontAtlas.ConfigData[0].FontNo);
							if (stbtt_InitFont(&fontInfos, (unsigned char*)fontPtr->m_ImFontAtlas.ConfigData[0].FontData, font_offset))
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
					int32_t res = stbi_write_png(
						vFilePathName.c_str(),
						finalWidth + padding_x,
						finalHeight,
						1,
						buffer.data(),
						bufferWidth);

					if (res)
					{
						FileHelper::Instance()->OpenFile(vFilePathName);
					}
					else
					{
						Messaging::Instance()->AddError(true, 0, 0,
							"Png Writing Fail for path : %s", vFilePathName.c_str());
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0,
						"Png Writing Fail for path : %s, final computed size not ok %i, %i\n",
						vFilePathName.c_str(), finalWidth, finalHeight);
				}
			}
		}
	}
}

static std::string GetNewHeaderName(const std::string& vPrefix, const std::string& vName)
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
void Generator::GenerateCard_One(
	const std::string& vFilePathName,
	FontInfos* vFontInfos,
	const uint32_t& vGlyphHeight,	// max height of glyph
	const uint32_t& vMaxRows)		// max row count
{
	if (!vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			// file
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, ".png");
			
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
					glyphs[GetNewHeaderName(prefix, glyph.second)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)vFontInfos);
				}
			}
			else
			{
				for (auto& glyph : vFontInfos->m_SelectedGlyphs)
				{
					glyphs[GetNewHeaderName(prefix, glyph.second.newHeaderName)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)vFontInfos);
				}
			}

			WriteGlyphCardToPicture(filePathName, glyphs, vGlyphHeight, vMaxRows);
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0, "Invalid path : %s", vFilePathName.c_str());
		}
	}
}

void Generator::GenerateCard_Merged(
	const std::string& vFilePathName,
	ProjectFile* vProjectFile,
	const uint32_t& vGlyphHeight,	// max height of glyph
	const uint32_t& vMaxRows)		// max row count
{
	UNUSED(vFilePathName);
	UNUSED(vProjectFile);
	UNUSED(vGlyphHeight);
	UNUSED(vMaxRows);

	if (vProjectFile &&
		!vFilePathName.empty() &&
		!vProjectFile->m_Fonts.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			// file
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, ".png");

			// Prefix
			std::string prefix = "";
			if (!vProjectFile->m_MergedFontPrefix.empty())
				prefix = "ICON_" + vProjectFile->m_MergedFontPrefix;

			// generate bd of old codepoint and new string
			// old codepoint because, we will get the glyph from te already loaded texture,
			// so can only be accessed with old codepoints
			std::map<std::string, std::pair<uint32_t, size_t>> glyphs;
			for (const auto& font : vProjectFile->m_Fonts)
			{
				for (const auto& glyph : font.second.m_SelectedGlyphs)
				{
					glyphs[GetNewHeaderName(prefix, glyph.second.newHeaderName)] = std::pair<uint32_t, size_t>(glyph.first, (size_t)&font.second);
				}
			}

			WriteGlyphCardToPicture(filePathName, glyphs, vGlyphHeight, vMaxRows);
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0, "Invalid path : %s", vFilePathName.c_str());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// HEADER GENERATION ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* 03/03/2020 22h20 it work like a charm for the two modes */
/*
Generate Header file with glyphs code like in https://github.com/juliettef/IconFontCppHeaders
two modes :
- if not glyph were selected => will generate header for all font glyphs
- if glyph were selected => will generate header only for these glyphs
*/
void Generator::GenerateHeader_One(
	const std::string& vFilePathName, 
	FontInfos *vFontInfos, 
	std::string vFontBufferName, // for header generation wehn using a cpp bytes array instead of a file
	size_t vFontBufferSize) // for header generation wehn using a cpp bytes array instead of a file
{
	if (!vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string header;
			header += "//Header Generated with https://github.com/aiekick/ImGuiFontStudio\n";
			header += "//Based on https://github.com/juliettef/IconFontCppHeaders\n";
			header += "\n#pragma once\n\n";

			std::string prefix = "";
			if (!vFontInfos->m_FontPrefix.empty())
				prefix = "_" + vFontInfos->m_FontPrefix;
			if (vFontBufferSize > 0)
			{
				header += "#define FONT_ICON_BUFFER_NAME" + prefix + " " + vFontBufferName + "\n";
				header += "#define FONT_ICON_BUFFER_SIZE" + prefix + " 0x" + ct::toHexStr(vFontBufferSize) + "\n\n";
			}
			else
			{
				header += "#define FONT_ICON_FILE_NAME" + prefix + " \"" + vFontInfos->m_FontFileName + "\"\n\n";
			}

			uint32_t minCodePoint = 65535;
			uint32_t maxCodePoint = 0;
			std::string glyphs;

			std::map<std::string, uint32_t> finalGlyphNames;
			
			std::map<std::string, uint32_t> glyphNames;

			if (vFontInfos->m_SelectedGlyphs.empty()) // no glyph selected so generate for whole font
			{
				for (auto& it : vFontInfos->m_GlyphCodePointToName)
				{
					glyphNames[it.second] = it.first;
				}
			}
			else
			{
				for (auto& it : vFontInfos->m_SelectedGlyphs)
				{
					glyphNames[it.second.newHeaderName] = it.second.newCodePoint;
				}
			}

			// convert first and let in map.
			// like that they will be ordered by names
			// the issue was to convert and write directly
			// so the original tag "SetMode" was writen before the original tag "about".
			// its not wanted, so we save and order the finla result we want
			// and after we will write in header file
			for (const auto& it : glyphNames)
			{
				std::string glyphName = it.first;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				uint32_t codePoint = it.second;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto& c : glyphName)
					c = (char)toupper((int32_t)c);

				finalGlyphNames[glyphName] = codePoint;
			}

			for (const auto& it : finalGlyphNames)
			{
				glyphs += "#define ICON" + prefix + "_" + it.first + " u8\"\\u" + ct::toHexStr(it.second) + "\"\n";
			}
			
			// code point range
			header += "#define ICON_MIN" + prefix + " 0x" + ct::toHexStr(minCodePoint) + "\n";
			header += "#define ICON_MAX" + prefix + " 0x" + ct::toHexStr(maxCodePoint) + "\n\n";

			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, ".h");

			FileHelper::Instance()->SaveStringToFile(header + glyphs, filePathName);
			FileHelper::Instance()->OpenFile(filePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0, "Invalid path : %s", vFilePathName.c_str());
		}
	}
}

void Generator::GenerateHeader_Merged(
	const std::string& vFilePathName,
	ProjectFile* vProjectFile,
	std::string vFontBufferName, // for header generation wehn using a cpp bytes array instead of a file
	size_t vFontBufferSize) // for header generation wehn using a cpp bytes array instead of a file
{
	if (vProjectFile &&
		!vFilePathName.empty() &&
		!vProjectFile->m_Fonts.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string header;
			header += "//Header Generated with https://github.com/aiekick/ImGuiFontStudio\n";
			header += "//Based on https://github.com/juliettef/IconFontCppHeaders\n";
			header += "\n#pragma once\n\n";

			std::string prefix = "";
			if (!vProjectFile->m_MergedFontPrefix.empty())
				prefix = "_" + vProjectFile->m_MergedFontPrefix;
			if (vFontBufferSize > 0)
			{
				header += "#define FONT_ICON_BUFFER_NAME" + prefix + " " + vFontBufferName + "\n";
				header += "#define FONT_ICON_BUFFER_SIZE" + prefix + " 0x" + ct::toHexStr(vFontBufferSize) + "\n\n";
			}
			else
			{
				header += "#define FONT_ICON_FILE_NAME" + prefix + " \"" + ps.name + "." + ps.ext + "\"\n\n";
			}

			uint32_t minCodePoint = 65535;
			uint32_t maxCodePoint = 0;
			std::string glyphs;

			std::map<std::string, uint32_t> finalGlyphNames;

			std::map<std::string, uint32_t> glyphNames;

			// we take only selected glyphs of all fonts
			for (const auto& font : vProjectFile->m_Fonts)
			{
				for (const auto& glyph : font.second.m_SelectedGlyphs)
				{
					glyphNames[glyph.second.newHeaderName] = glyph.second.newCodePoint;
				}
			}

			// convert first and let in map.
			// like that they will be ordered by names
			// the issue was to convert and write directly
			// so the original tag "SetMode" was writen before the original tag "about".
			// its not wanted, so we save and order the finla result we want
			// and after we will write in header file
			for (const auto& it : glyphNames)
			{
				std::string glyphName = it.first;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				uint32_t codePoint = it.second;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto& c : glyphName)
					c = (char)toupper((int32_t)c);

				finalGlyphNames[glyphName] = codePoint;
			}

			for (const auto& it : finalGlyphNames)
			{
				glyphs += "#define ICON" + prefix + "_" + it.first + " u8\"\\u" + ct::toHexStr(it.second) + "\"\n";
			}

			// code point range
			header += "#define ICON_MIN" + prefix + " 0x" + ct::toHexStr(minCodePoint) + "\n";
			header += "#define ICON_MAX" + prefix + " 0x" + ct::toHexStr(maxCodePoint) + "\n\n";

			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, ".h");
			
			FileHelper::Instance()->SaveStringToFile(header + glyphs, filePathName);
			FileHelper::Instance()->OpenFile(filePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0, "Invalid path : %s", vFilePathName.c_str());
		}
	}
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
void Generator::GenerateFontFile_One(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	FontInfos *vFontInfos,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && !vFilePathName.empty() && vFontInfos)
	{
		FontHelper fontHelper;

		bool res = false;

		std::map<int32_t, std::string> newHeaderNames;
		std::map<int32_t, int32_t> newCodePoints;
		std::map<CodePoint, GlyphInfos> newGlyphInfos;
		for (const auto &glyph : vFontInfos->m_SelectedGlyphs)
		{
			newHeaderNames[glyph.first] = glyph.second.newHeaderName;
			newCodePoints[glyph.first] = glyph.second.newCodePoint;
			newGlyphInfos[glyph.first] = glyph.second;
		}

		if (!newHeaderNames.empty() && !newCodePoints.empty())
		{
			std::string absPath = vProjectFile->GetAbsolutePath(vFontInfos->m_FontFilePathName);
			res = fontHelper.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, true);
		}

		if (res)
		{
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
				std::string name = ps.name;
				ct::replaceString(name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt(name, ".ttf");
				
				if (fontHelper.GenerateFontFile(filePathName, vFlags & GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
				{
					if (vFlags & GENERATOR_MODE_HEADER)
					{
						GenerateHeader_One(
							filePathName, 
							vFontInfos);
					}
					if (vFlags & GENERATOR_MODE_CARD)
					{
						GenerateCard_One(
							filePathName, 
							vFontInfos, 
							vProjectFile->m_CardGlyphHeightInPixel, 
							vProjectFile->m_CardCountRowsMax);
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0, "Cannot create font file %s", filePathName.c_str());
					return;
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0, "File Path Name is wrong : %s", vFilePathName.c_str());
				return;
			}
			
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Could not open font file %s.\n", 
				vProjectFile->m_SelectedFont->m_FontFilePathName.c_str());
			return;
		}
	}
}

void Generator::GenerateFontFile_Merged(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && vProjectFile->IsLoaded() &&
		!vFilePathName.empty() &&
		!vProjectFile->m_Fonts.empty() &&
		!vProjectFile->m_FontToMergeIn.empty())
	{
		FontHelper fontHelper;

		bool res = true;

		ct::ivec2 baseSize = 0;
		ct::ivec4 baseFontBoundingBox;
		int32_t baseFontAscent = 0;
		int32_t baseFontDescent = 0;

		// abse infos for merge all toher fonts in this one
		for (auto &it : vProjectFile->m_Fonts)
		{
			if (vProjectFile->m_FontToMergeIn == it.second.m_FontFileName &&
				!vProjectFile->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
			{
				baseFontBoundingBox = it.second.m_BoundingBox;
				baseFontAscent = it.second.m_Ascent;
				baseFontDescent = it.second.m_Descent;
				baseSize = it.second.m_BoundingBox.zw() - it.second.m_BoundingBox.xy();
			}
		}

		for (auto &it : vProjectFile->m_Fonts)
		{
			bool scaleChanged = false;
			ct::dvec2 scale = 1.0;
			if (vProjectFile->m_FontToMergeIn != it.second.m_FontFileName &&
				!vProjectFile->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
			{
				scaleChanged = true;
				ct::ivec2 newSize = it.second.m_BoundingBox.zw() - it.second.m_BoundingBox.xy();
				scale.x = (double)baseSize.x / (double)newSize.x;
				scale.y = (double)baseSize.y / (double)newSize.y;
				double v = ct::mini(scale.x, scale.y);
				scale.x = v; // same value for keep glyph ratio
				scale.y = v; // same value for keep glyph ratio
			}

			std::map<int32_t, std::string> newHeaderNames;
			std::map<int32_t, int32_t> newCodePoints;
			std::map<CodePoint, GlyphInfos> newGlyphInfos;
			for (const auto &glyph : it.second.m_SelectedGlyphs)
			{
				GlyphInfos gInfos = glyph.second;

				newHeaderNames[glyph.first] = gInfos.newHeaderName;
				newCodePoints[glyph.first] = gInfos.newCodePoint;

				if (scaleChanged &&
					!vProjectFile->IsGenMode(GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE))
				{
					gInfos.simpleGlyph.isValid = true;
					gInfos.simpleGlyph.m_Scale = scale;

					gInfos.m_FontBoundingBox = baseFontBoundingBox;
					gInfos.m_FontAscent = baseFontAscent;
					gInfos.m_FontDescent = baseFontDescent;
				}
								
				newGlyphInfos[glyph.first] = gInfos;
			}

			if (!newHeaderNames.empty())
			{
				std::string absPath = vProjectFile->GetAbsolutePath(it.second.m_FontFilePathName);
				res &= fontHelper.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, !scaleChanged);
			}
			else
			{
				res = false;
			}
		}
		
		if (res)
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
				filePathName = ps.GetFPNE_WithNameExt(name, ".ttf");

				if (fontHelper.GenerateFontFile(vFilePathName, vFlags & GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
				{
					if (vFlags & GENERATOR_MODE_HEADER)
					{
						GenerateHeader_Merged(
							filePathName, 
							vProjectFile);
					}
					if (vFlags & GENERATOR_MODE_CARD)
					{
						GenerateCard_Merged(
							filePathName, 
							vProjectFile,
							vProjectFile->m_CardGlyphHeightInPixel, 
							vProjectFile->m_CardCountRowsMax);
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0, "Cannot create font file %s", filePathName.c_str());
					return;
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0, "File Path Name is wrong : %s", vFilePathName.c_str());
				return;
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Could not open font file %s.\n",
				vProjectFile->m_SelectedFont->m_FontFilePathName.c_str());
			return;
		}
	}
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
void Generator::GenerateCpp_One(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	FontInfos *vFontInfos,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && !vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string res;

			bool generateTemporaryFontFile = false;
			if (vFontInfos->m_SelectedGlyphs.empty()) // export whole font file to cpp (and header is asked)
			{
				filePathName = vProjectFile->GetAbsolutePath(vFontInfos->m_FontFilePathName);
				// todo: il faut gerer le cas ou on va exporter tout les fichier sans selction
				// ici on ne fait que l'actif, c'est aps ce qu'on veut
			}
			else // export glyph selection in a new temporary font file, then exported in cpp (and header if asked)
			{
				// generate temporary font file first
				generateTemporaryFontFile = true;
				
				std::string name = ps.name;
				ct::replaceString(name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt("temporary_" + name, ".ttf");

				GenerateFontFile_One(filePathName, vProjectFile, vFontInfos,
					(GenModeFlags)(vFlags & ~GENERATOR_MODE_HEADER_CARD)); // no header or card to generate
			}

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				std::string bufferName;
				size_t bufferSize = 0;
				res = Compress::GetCompressedBase85BytesArray(
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
				if (!res.empty() && !bufferName.empty() && bufferSize > 0)
				{
					filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".cpp";

					if (vFlags & GENERATOR_MODE_HEADER)
					{
						res = "#include \"" + ps.name + ".h\"\n\n" + res;
						std::string prefix = "";
						if (!vFontInfos->m_FontPrefix.empty())
							prefix = "_" + vFontInfos->m_FontPrefix;
						prefix = "FONT_ICON_BUFFER_NAME" + prefix;
						ct::replaceString(res, vFontInfos->m_FontPrefix + "_compressed_data_base85", prefix);

						GenerateHeader_One(
							filePathName, 
							vFontInfos, 
							bufferName, 
							bufferSize);
					}

					if (vFlags & GENERATOR_MODE_CARD)
					{
						GenerateCard_One(
							filePathName, 
							vFontInfos, 
							vProjectFile->m_CardGlyphHeightInPixel, 
							vProjectFile->m_CardCountRowsMax);
					}

					FileHelper::Instance()->SaveStringToFile(res, filePathName);
					FileHelper::Instance()->OpenFile(filePathName);
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0,
						"Error opening or reading file %s", filePathName.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0,
					"Cant open file %s", filePathName.c_str());
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Bad File path Name %s", filePathName.c_str());
		}
	}
}

void Generator::GenerateCpp_Merged(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	const GenModeFlags& vFlags)
{
	if (vProjectFile &&	!vFilePathName.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string res;

			ct::replaceString(ps.name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt("temporary_" + ps.name, ".ttf");

			GenerateFontFile_Merged(
				filePathName, 
				vProjectFile,
				(GenModeFlags)(vFlags & ~GENERATOR_MODE_HEADER_CARD)); // no header to generate

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				std::string bufferName;
				size_t bufferSize = 0;
				res = Compress::GetCompressedBase85BytesArray(
					filePathName,
					vProjectFile->m_MergedFontPrefix,
					&bufferName,
					&bufferSize);

				// we have the result, if empty or not we need to destroy the temporary font file
				FileHelper::Instance()->DestroyFile(filePathName);

				// if ok, serialization
				if (!res.empty() && !bufferName.empty() && bufferSize > 0)
				{
					filePathName = ps.GetFPNE_WithNameExt(ps.name, ".cpp");

					if (vFlags & GENERATOR_MODE_HEADER)
					{
						res = "#include \"" + ps.name + ".h\"\n\n" + res;
						std::string prefix = "";
						if (!vProjectFile->m_MergedFontPrefix.empty())
							prefix = "_" + vProjectFile->m_MergedFontPrefix;
						prefix = "FONT_ICON_BUFFER_NAME" + prefix;
						ct::replaceString(res, vProjectFile->m_MergedFontPrefix + "_compressed_data_base85", prefix);

						GenerateHeader_Merged(
							filePathName, 
							vProjectFile, 
							bufferName, 
							bufferSize);
					}

					if (vFlags & GENERATOR_MODE_CARD)
					{
						GenerateCard_Merged(
							filePathName, 
							vProjectFile, 
							vProjectFile->m_CardGlyphHeightInPixel, 
							vProjectFile->m_CardCountRowsMax);
					}

					FileHelper::Instance()->SaveStringToFile(res, filePathName);
					FileHelper::Instance()->OpenFile(filePathName);
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0,
						"Error opening or reading file %s", filePathName.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0,
					"Cant open file %s", filePathName.c_str());
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Bad File path Name %s", filePathName.c_str());
		}
	}
}

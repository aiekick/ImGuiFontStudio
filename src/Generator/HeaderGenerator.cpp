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

#include "HeaderGenerator.h"

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
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>

///////////////////////////////////////////////////////////////////////////////////
//// HEADER GENERATION ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static std::string GetHeader(std::string vLang, std::string vPrefix)
{
	std::string header;
	header += "//Header Generated with https://github.com/aiekick/ImGuiFontStudio\n";
	header += "//Based on https://github.com/juliettef/IconFontCppHeaders\n";
	
	if (vLang == "c")
	{
		header += "//for Language c\n";
		header += "\n";
		header += ct::toStr("#ifndef %s_FONT_ICON_H\n", vPrefix.c_str());
		header += ct::toStr("#define %s_FONT_ICON_H\n\n", vPrefix.c_str());
	}
	else if (vLang == "cpp")
	{
		header += "//for Language c++\n";
		header += "\n";
		header += ct::toStr("#pragma once\n\n");
	}
	else if (vLang == "c#")
	{
		header += "//for Language c#\n\n";
		header += "namespace IconFonts\n";
		header += "{\n";
		header += ct::toStr("\tpublic class %s_Labels\n", vPrefix.c_str());
		header += "\t{\n";
	}

	return header;
}

static std::string GetFooter(std::string vLang, std::string vPrefix)
{
	std::string header;

	if (vLang == "c#")
	{
		header += "\t}\n";
		header += "}\n";
	}
	else if (vLang == "c")
	{
		header += ct::toStr("\n#endif // %s_FONT_ICON_H\n", vPrefix.c_str());
	}
	return header;
}

static std::string GetFontInfos(std::string vLang, std::string vPrefix, std::string vFontFileName, std::string vFontBufferName, size_t vFontBufferSize)
{
	std::string header;

	if (vFontBufferSize > 0)
	{
		if (vLang == "cpp" || 
			vLang == "c")
		{
			header += ct::toStr("#define FONT_ICON_BUFFER_NAME_%s %s\n", vPrefix.c_str(), vFontBufferName.c_str());
			header += ct::toStr("#define FONT_ICON_BUFFER_SIZE_%s 0x%s\n", vPrefix.c_str(), ct::toHexStr(vFontBufferSize).c_str());
		}
		else if (vLang == "c#")
		{
			//header += ct::toStr("\t\tpublic const int FONT_ICON_ARRAY_SIZE = 0x%s;\n", ct::toHexStr(vFontBufferSize).c_str());
		}
	}
	else
	{
		if (vLang == "cpp" || 
			vLang == "c")
		{
			header += ct::toStr("#define FONT_ICON_FILE_NAME_%s \"%s\"\n", vPrefix.c_str(), vFontFileName.c_str());
		}
		else if (vLang == "c#")
		{
			header += ct::toStr("\t\tpublic const string FONT_ICON_FILE_NAME = \"%s\";\n", vFontFileName.c_str());
		}
	}

	header += "\n";

	return header;
}

static std::string GetGlyphTableMinMax(std::string vLang, std::string vPrefix, ct::uvec2 vCodePointRange)
{
	std::string header;

	if (vLang == "cpp" || 
		vLang == "c")
	{
		header += ct::toStr("#define ICON_%s_MIN 0x%s\n", vPrefix.c_str(), ct::toHexStr(vCodePointRange.x).c_str());
		header += ct::toStr("#define ICON_%s_MAX 0x%s\n", vPrefix.c_str(), ct::toHexStr(vCodePointRange.y).c_str());
	}
	else if (vLang == "c#")
	{
		header += ct::toStr("\t\tpublic const int ICON_MIN = 0x%s;\n", ct::toHexStr(vCodePointRange.x).c_str());
		header += ct::toStr("\t\tpublic const int ICON_MAX = 0x%s;\n", ct::toHexStr(vCodePointRange.y).c_str());
	}

	header += "\n";

	return header;
}

static std::string GetGlyphItem(std::string vLang, std::string vType, std::string vPrefix, std::string vLabel, uint32_t vCodePoint)
{
	std::string header;
	
	if (vLang == "cpp" || 
		vLang == "c")
	{
		header += ct::toStr("#define %s_%s_%s u8\"\\u%s\"\n", vType.c_str(), vPrefix.c_str(), vLabel.c_str(), ct::toHexStr(vCodePoint).c_str());
	}
	else if (vLang == "c#")
	{
		header += ct::toStr("\t\tpublic const string %s_%s = \"\\u%s\";\n", vType.c_str(), vLabel.c_str(), ct::toHexStr(vCodePoint).c_str());
	}

	return header;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string HeaderGenerator::GenerateHeaderFile(std::string vLang, std::string vPrefix, std::string vFontFileName, std::string vFontBufferName, size_t vFontBufferSize)
{
	std::string headerFile;
	headerFile += GetHeader(vLang, vPrefix);
	headerFile += GetFontInfos(vLang, vPrefix, vFontFileName, vFontBufferName, vFontBufferSize);
	headerFile += GetGlyphTableMinMax(vLang, vPrefix, m_FinalCodePointRange);
	for (const auto& it : m_FinalGlyphNames)
	{
		headerFile += GetGlyphItem(vLang, "ICON", vPrefix, it.first, it.second);
	}
	headerFile += GetFooter(vLang, vPrefix);
	return headerFile;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string GetNewHeaderName(const std::string& vName)
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

	return glyphName;
}

// 03/01/2020 22h20 it work like a charm for the two modes
/*
Generate Header file with glyphs code like in https://github.com/juliettef/IconFontCppHeaders
two modes :
- if not glyph were selected => will generate header for all font glyphs
- if glyph were selected => will generate header only for these glyphs
*/
void HeaderGenerator::GenerateHeader_One(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	std::shared_ptr<FontInfos> vFontInfos,
	std::string vFontBufferName, // for header generation wehn using a cpp bytes array instead of a file
	size_t vFontBufferSize) // for header generation wehn using a cpp bytes array instead of a file
{
	if (!vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::map<std::string, uint32_t> glyphNames;
			if (vFontInfos->m_SelectedGlyphs.empty()) // no glyph selected so generate for whole font
				for (auto& it : vFontInfos->m_GlyphCodePointToName)
					glyphNames[it.second] = it.first;
			else
				for (auto& it : vFontInfos->m_SelectedGlyphs)
					if (it.second)
						glyphNames[it.second->newHeaderName] = it.second->newCodePoint;

			m_FinalGlyphNames.clear();
			m_FinalCodePointRange = ct::uvec2(65535, 0);
			for (const auto& it : glyphNames)
			{
				m_FinalCodePointRange.x = ct::mini(m_FinalCodePointRange.x, it.second);
				m_FinalCodePointRange.y = ct::maxi(m_FinalCodePointRange.y, it.second);
				m_FinalGlyphNames[GetNewHeaderName(it.first)] = it.second;
			}

			std::string lang, headerExt;
			if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_C))
			{
				lang = "c";
				headerExt = ".h";
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CPP))
			{
				lang = "cpp";
				headerExt = ".h";
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
			{
				lang = "c#";
				headerExt = ".cs";
			}
			if (!lang.empty())
			{
				std::string name = ps.name;
				ct::replaceString(name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt(name, headerExt);

				/////////////////////
				// header generation
				std::string headerFile = GenerateHeaderFile(
					lang, vFontInfos->m_FontPrefix,
					vFontInfos->m_FontFileName,
					vFontBufferName, vFontBufferSize);
				FileHelper::Instance()->SaveStringToFile(headerFile, filePathName);
				/////////////////////
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr, "Language not set for : %s", vFilePathName.c_str());
			}

			FileHelper::Instance()->OpenFile(filePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr, "Invalid path : %s", vFilePathName.c_str());
		}
	}
}

void HeaderGenerator::GenerateHeader_Merged(
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
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFPNE_WithNameExt(name, ".h");
			
			// we take only selected glyphs of all fonts
			std::map<std::string, uint32_t> glyphNames;
			for (const auto font : vProjectFile->m_Fonts)
				for (const auto& glyph : font.second->m_SelectedGlyphs)
					if (glyph.second)
						glyphNames[glyph.second->newHeaderName] = glyph.second->newCodePoint;
					
			m_FinalGlyphNames.clear();
			m_FinalCodePointRange = ct::uvec2(65535, 0);
			for (const auto& it : glyphNames)
			{
				m_FinalCodePointRange.x = ct::mini(m_FinalCodePointRange.x, it.second);
				m_FinalCodePointRange.y = ct::maxi(m_FinalCodePointRange.y, it.second);
				m_FinalGlyphNames[GetNewHeaderName(it.first)] = it.second;
			}

			std::string lang, headerExt;
			if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_C))
			{
				lang = "c";
				headerExt = ".h";
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CPP))
			{
				lang = "cpp";
				headerExt = ".h";
			}
			else if (vProjectFile->IsGenMode(GENERATOR_MODE_LANG_CSHARP))
			{
				lang = "c#";
				headerExt = ".cs";
			}
			if (!lang.empty())
			{
				std::string _name = ps.name;
				ct::replaceString(_name, "-", "_");
				filePathName = ps.GetFPNE_WithNameExt(_name, headerExt);

				/////////////////////
				// header generation
				std::string headerFile = GenerateHeaderFile(
					lang, vProjectFile->m_MergedFontPrefix,
					ps.name + "." + ps.ext,
					vFontBufferName, vFontBufferSize);
				FileHelper::Instance()->SaveStringToFile(headerFile, filePathName);
				/////////////////////
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr, "Language not set for : %s", vFilePathName.c_str());
			}

			FileHelper::Instance()->OpenFile(filePathName);
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr, "Invalid path : %s", vFilePathName.c_str());
		}
	}
}

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

#include <GLFW/glfw3.h>
#include <ctools/cTools.h>

#include "HeaderGenerator.h"

#include <stdint.h>
#include <string>

typedef int GenModeFlags;
enum _GenModeFlags
{
	GENERATOR_MODE_NONE = 0,
	GENERATOR_MODE_HEADER = (1 << 0),	// header .h
	GENERATOR_MODE_CARD = (1 << 1),		// picture of glyph icon and labels
	GENERATOR_MODE_FONT = (1 << 2),		// ttf file
	GENERATOR_MODE_SRC = (1 << 3),		// src embedded file (cpp, c#, etc)
	GENERATOR_MODE_CURRENT = (1 << 4),
	GENERATOR_MODE_BATCH = (1 << 5),
	GENERATOR_MODE_MERGED = (1 << 6),
	GENERATOR_MODE_MERGED_SETTINGS_DISABLE_GLYPH_RESCALE = (1 << 7),
	GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES = (1 << 8),
	GENERATOR_MODE_LANG_C = (1 << 9),
	GENERATOR_MODE_LANG_CPP = (1 << 10),
	GENERATOR_MODE_LANG_CSHARP = (1 << 11),
	GENERATOR_MODE_LANG_LUA = (1 << 12),
	GENERATOR_MODE_LANG_PYTHON = (1 << 13),
	GENERATOR_MODE_LANG_RUST = (1 << 14),
	GENERATOR_MODE_OPEN_GENERATED_FILES_AUTO = (1 << 15),

	// Mix's

	 // for radio widget's
	GENERATOR_MODE_RADIO_LANG = GENERATOR_MODE_LANG_C | GENERATOR_MODE_LANG_CPP | GENERATOR_MODE_LANG_CSHARP | GENERATOR_MODE_LANG_LUA | GENERATOR_MODE_LANG_PYTHON | GENERATOR_MODE_LANG_RUST,
	GENERATOR_MODE_RADIO_FONT_SRC = GENERATOR_MODE_FONT | GENERATOR_MODE_SRC,
	GENERATOR_MODE_RADIO_CUR_BAT_MER = GENERATOR_MODE_CURRENT | GENERATOR_MODE_BATCH | GENERATOR_MODE_MERGED,
	
	// for group's
	
	// header
	GENERATOR_MODE_HEADER_CARD = GENERATOR_MODE_HEADER | GENERATOR_MODE_CARD,
	GENERATOR_MODE_HEADER_CARD_SRC = GENERATOR_MODE_HEADER | GENERATOR_MODE_CARD | GENERATOR_MODE_SRC,

	// font alone
	GENERATOR_MODE_CURRENT_CARD = GENERATOR_MODE_CURRENT | GENERATOR_MODE_CARD,
	GENERATOR_MODE_CURRENT_HEADER = GENERATOR_MODE_CURRENT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_CURRENT_HEADER_CARD = GENERATOR_MODE_CURRENT | GENERATOR_MODE_HEADER_CARD,
	GENERATOR_MODE_CURRENT_FONT = GENERATOR_MODE_CURRENT | GENERATOR_MODE_FONT,
	GENERATOR_MODE_CURRENT_SRC = GENERATOR_MODE_CURRENT | GENERATOR_MODE_SRC,
	GENERATOR_MODE_CURRENT_FONT_HEADER = GENERATOR_MODE_CURRENT_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_CURRENT_SRC_HEADER = GENERATOR_MODE_CURRENT_SRC | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_CARD = GENERATOR_MODE_BATCH | GENERATOR_MODE_CARD,

	// batch
	GENERATOR_MODE_BATCH_HEADER = GENERATOR_MODE_BATCH | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_FONT = GENERATOR_MODE_BATCH | GENERATOR_MODE_FONT,
	GENERATOR_MODE_BATCH_SRC = GENERATOR_MODE_BATCH | GENERATOR_MODE_SRC,
	GENERATOR_MODE_BATCH_FONT_HEADER = GENERATOR_MODE_BATCH_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_SRC_HEADER = GENERATOR_MODE_BATCH_SRC | GENERATOR_MODE_HEADER,

	// merged
	GENERATOR_MODE_MERGED_CARD = GENERATOR_MODE_MERGED | GENERATOR_MODE_CARD,
	GENERATOR_MODE_MERGED_HEADER = GENERATOR_MODE_MERGED | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_FONT = GENERATOR_MODE_MERGED | GENERATOR_MODE_FONT,
	GENERATOR_MODE_MERGED_SRC = GENERATOR_MODE_MERGED | GENERATOR_MODE_SRC,
	GENERATOR_MODE_MERGED_FONT_HEADER = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_SRC_HEADER = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_FONT_HEADER_CARD = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER | GENERATOR_MODE_CARD,
	GENERATOR_MODE_MERGED_SRC_HEADER_CARD = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER | GENERATOR_MODE_CARD
};

class FontInfos;
class ProjectFile;
class Generator
{
public:
	static bool SaveTextureToPng(GLFWwindow* vWin, const std::string& vFilePathName,
		GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount);
	static bool WriteGlyphCardToPicture(
		const std::string& vFilePathName,
		std::map<std::string, std::pair<uint32_t, size_t>> vLabels, // lable, codepoint, FontInfos ptr
		const uint32_t& vGlyphHeight, const uint32_t& vMaxRows);

private:
	HeaderGenerator m_HeaderGenerator;

public:
	bool Generate(
		ProjectFile *vProjectFile,
		const std::string& vFilePath = "",
		const std::string& vFileName = "");

private:
	bool GenerateCard_One(const std::string& vFilePathName, std::shared_ptr<FontInfos> vFontInfos, ProjectFile* vProjectFile);
	bool GenerateCard_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile);
	
	/*void GenerateHeader_One(const std::string& vFilePathName, std::shared_ptr<FontInfos> vFontInfos,
		std::string vFontBufferName = "", size_t vFontBufferSize = 0);
	void GenerateHeader_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		std::string vFontBufferName = "", size_t vFontBufferSize = 0);*/
	
	bool GenerateFontFile_One(const std::string& vFilePathName, ProjectFile* vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, const GenModeFlags& vFlags);
	bool GenerateFontFile_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const GenModeFlags& vFlags);
	
	bool GenerateSource_One(const std::string& vFilePathName, ProjectFile* vProjectFile,
		std::shared_ptr<FontInfos> vFontInfos, const GenModeFlags& vFlags);
	bool GenerateSource_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const GenModeFlags& vFlags);

public: // singleton
	static Generator *Instance()
	{
		static Generator *_instance = new Generator();
		return _instance;
	}

protected:
	Generator(); // Prevent construction
	Generator(const Generator&) {}; // Prevent construction by copying
	Generator& operator =(const Generator&) { return *this; }; // Prevent assignment
	~Generator(); // Prevent unwanted destruction};
};


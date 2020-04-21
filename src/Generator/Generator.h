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

#include <stdint.h>
#include <string>
#include <map>

typedef int GenModeFlags;
enum _GenModeFlags
{
	GENERATOR_MODE_NONE = 0,
	GENERATOR_MODE_HEADER = (1 << 0),
	GENERATOR_MODE_FONT = (1 << 1),
	GENERATOR_MODE_CPP = (1 << 2),
	GENERATOR_MODE_CURRENT = (1 << 3),
	GENERATOR_MODE_BATCH = (1 << 4),
	GENERATOR_MODE_MERGED = (1 << 5),
	GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES = (1 << 6),
	GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_CODEPOINT = (1 << 7),
	GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES = (1 << 8),
	GENERATOR_MODE_RADIO_FONT_CPP = GENERATOR_MODE_FONT | GENERATOR_MODE_CPP, // for raido widgets
	GENERATOR_MODE_RADIO_CUR_BAT_MER = GENERATOR_MODE_CURRENT | GENERATOR_MODE_BATCH | GENERATOR_MODE_MERGED, // for radio widgets
	GENERATOR_MODE_RADIO_CDP_NAMES = GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_CODEPOINT | GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES, // for radio widgets
	GENERATOR_MODE_CURRENT_HEADER = GENERATOR_MODE_CURRENT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_CURRENT_FONT = GENERATOR_MODE_CURRENT | GENERATOR_MODE_FONT,
	GENERATOR_MODE_CURRENT_CPP = GENERATOR_MODE_CURRENT | GENERATOR_MODE_CPP,
	GENERATOR_MODE_CURRENT_FONT_HEADER = GENERATOR_MODE_CURRENT_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_CURRENT_CPP_HEADER = GENERATOR_MODE_CURRENT_CPP | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_HEADER = GENERATOR_MODE_BATCH | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_FONT = GENERATOR_MODE_BATCH | GENERATOR_MODE_FONT,
	GENERATOR_MODE_BATCH_CPP = GENERATOR_MODE_BATCH | GENERATOR_MODE_CPP,
	GENERATOR_MODE_BATCH_FONT_HEADER = GENERATOR_MODE_BATCH_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_BATCH_CPP_HEADER = GENERATOR_MODE_BATCH_CPP | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_HEADER = GENERATOR_MODE_MERGED | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_FONT = GENERATOR_MODE_MERGED | GENERATOR_MODE_FONT,
	GENERATOR_MODE_MERGED_CPP = GENERATOR_MODE_MERGED | GENERATOR_MODE_CPP,
	GENERATOR_MODE_MERGED_FONT_HEADER = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER,
	GENERATOR_MODE_MERGED_CPP_HEADER = GENERATOR_MODE_MERGED_FONT | GENERATOR_MODE_HEADER
};

class FontInfos;
class ProjectFile;
class Generator
{
public:
	void Generate(
		const std::string& vFilePath,
		const std::string& vFileName,
		ProjectFile* vProjectFile);
private:
	void GenerateHeader_One(const std::string& vFilePathName, FontInfos *vFontInfos,
		const GenModeFlags& vFlags, std::string vFontBufferName = "", size_t vFontBufferSize = 0);
	void GenerateHeader_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const GenModeFlags& vFlags, std::string vFontBufferName = "", size_t vFontBufferSize = 0);
	
	void GenerateFontFile_One(const std::string& vFilePathName, ProjectFile* vProjectFile, 
		FontInfos *vFontInfos, const GenModeFlags& vFlags);
	void GenerateFontFile_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const GenModeFlags& vFlags);
	
	void GenerateCpp_One(const std::string& vFilePathName, ProjectFile* vProjectFile,
		FontInfos *vFontInfos, const GenModeFlags& vFlags);
	void GenerateCpp_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const GenModeFlags& vFlags);

private: // cpp generation imported from ImGui
	std::string get_Compressed_Base85_BytesArray(const std::string& vFilePathName, 
		const std::string& vPrefix, std::string *vBufferName = 0, size_t *vBufferSize = 0);
	
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


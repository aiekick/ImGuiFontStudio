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

#include <Generator/GenMode.h>

class FontInfos;
class ProjectFile;
class Generator
{
public:
	static bool WriteGlyphCardToPicture(
		const std::string& vFilePathName,
		std::map<std::string, std::pair<uint32_t, size_t>> vLabels, // lable, codepoint, FontInfos ptr
		const uint32_t& vGlyphHeight, const uint32_t& vMaxRows);

private:
	HeaderGenerator m_HeaderGenerator;

public:
	bool Generate(
		const std::string& vFilePath = "",
		const std::string& vFileName = "");

private:
	bool GenerateCard_One(const std::string& vFilePathName, std::shared_ptr<FontInfos> vFontInfos);
	bool GenerateCard_Merged(const std::string& vFilePathName);
	
	/*void GenerateHeader_One(const std::string& vFilePathName, std::shared_ptr<FontInfos> vFontInfos,
		std::string vFontBufferName = "", size_t vFontBufferSize = 0);
	void GenerateHeader_Merged(const std::string& vFilePathName,
		std::string vFontBufferName = "", size_t vFontBufferSize = 0);*/
	
	bool GenerateFontFile_One(const std::string& vFilePathName,
		std::shared_ptr<FontInfos> vFontInfos, const GenModeFlags& vFlags);
	bool GenerateFontFile_Merged(const std::string& vFilePathName,
		const GenModeFlags& vFlags);
	
	bool GenerateSource_One(const std::string& vFilePathName,
		std::shared_ptr<FontInfos> vFontInfos, const GenModeFlags& vFlags);
	bool GenerateSource_Merged(const std::string& vFilePathName,
		const GenModeFlags& vFlags);

public: // singleton
	static Generator *Instance()
	{
		static Generator _instance;
		return &_instance;
	}

protected:
	Generator(); // Prevent construction
	Generator(const Generator&) {}; // Prevent construction by copying
	Generator& operator =(const Generator&) { return *this; }; // Prevent assignment
	~Generator(); // Prevent unwanted destruction};
};


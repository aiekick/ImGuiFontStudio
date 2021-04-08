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
#include <Generator/GenMode.h>

#include <stdint.h>
#include <string>

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
	bool GenerateCard_One(const std::string& vFilePathName, std::shared_ptr<FontInfos> vFontInfos,
		const uint32_t& vGlyphHeight, const uint32_t& vMaxRows);
	bool GenerateCard_Merged(const std::string& vFilePathName, ProjectFile* vProjectFile,
		const uint32_t& vGlyphHeight, const uint32_t& vMaxRows);
	
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


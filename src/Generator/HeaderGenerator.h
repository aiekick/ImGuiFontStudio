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

#include <Headers/Globals.h>
#include <ctools/cTools.h>

#include <stdint.h>
#include <string>
#include <memory>

class FontInfos;
class ProjectFile;
class HeaderGenerator
{
private:
	std::map<std::string, uint32_t> m_FinalGlyphNames;
	ct::uvec2 m_FinalCodePointRange = ct::uvec2(65535, 0);

public:
	void GenerateHeader_One(const std::string& vFilePathName,
		std::shared_ptr<FontInfos> vFontInfos, std::string vFontBufferName = "", size_t vFontBufferSize = 0);
	void GenerateHeader_Merged(const std::string& vFilePathName,
		std::string vFontBufferName = "", size_t vFontBufferSize = 0);

private:
	std::string GenerateHeaderFile(std::string vLang, std::string vPrefix, std::string vFontFileName, std::string vFontBufferName, size_t vFontBufferSize);
};


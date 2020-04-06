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
#include "GlyphInfos.h"

GlyphInfos::GlyphInfos()
{
	glyph.AdvanceX = 0;
	glyph.Codepoint = 0;
	glyph.U0 = 0;
	glyph.U1 = 0;
	glyph.V0 = 0;
	glyph.V1 = 0;
	glyph.Visible = false;
	glyph.X0 = 0;
	glyph.X1 = 0;
	glyph.Y0 = 0;
	glyph.Y1 = 0;
}

GlyphInfos::GlyphInfos(
	ImFontGlyph vGlyph, std::string vOldName, 
	std::string vNewName, ImWchar vNewCodePoint)
{
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
}

GlyphInfos::~GlyphInfos()
{

}
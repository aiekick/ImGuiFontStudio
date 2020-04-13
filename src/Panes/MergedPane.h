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

#include "Helper/FontHelper.h"

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

#include "sfntly/table/truetype/glyph_table.h"

class ProjectFile;
class FontInfos;
class GlyphInfos;
class MergedPane
{
private:
	//sfntly::Ptr<sfntly::GlyphTable::SimpleGlyph> m_SimpleGlyph;
	FontInstance m_fontInstance;
	GlyphInfos *m_Glyph = 0;
	GlyphInfos *m_Glyph2 = 0;

public:
	int DrawMergedPane(ProjectFile *vProjectFile, int vWidgetId);
	bool LoadGlyph(ProjectFile *vProjectFile, FontInfos* vFontInfos, GlyphInfos *vGlyphInfos);

private:
	bool DrawSimpleGlyph(GlyphInfos *vGlyph, FontInfos* vFontInfos,
		double vScale, double vProgress, bool vFill = false, bool vControlLines = true);

public: // singleton
	static MergedPane *Instance()
	{
		static MergedPane *_instance = new MergedPane();
		return _instance;
	}

protected:
	MergedPane(); // Prevent construction
	MergedPane(const MergedPane&) {}; // Prevent construction by copying
	MergedPane& operator =(const MergedPane&) { return *this; }; // Prevent assignment
	~MergedPane(); // Prevent unwanted destruction};
};


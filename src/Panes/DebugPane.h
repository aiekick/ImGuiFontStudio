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

#ifdef _DEBUG

#include <Panes/Abstract/AbstractPane.h>

#include <imgui/imgui.h>

#include <Project/GlyphInfos.h>

#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class FontInfos;
class DebugPane : public AbstractPane
{
private:
	std::weak_ptr<GlyphInfos> m_GlyphToDisplay;
	ct::ivec2 m_GlyphCurrentPoint = -1;

public:
	void Init() override;
	void Unit() override;
	int DrawPanes(ProjectFile* vProjectFile, int vWidgetId) override;
	void DrawDialogsAndPopups(ProjectFile* vProjectFile) override;
	int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas) override;

private:
	void DrawDebugPane(ProjectFile *vProjectFile);
	void DrawDebugGlyphPane(ProjectFile* vProjectFile);

public:
	void SetGlyphToDebug(std::weak_ptr<GlyphInfos> vGlyphInfos);
	void Clear();

	ct::ivec2 GetGlyphCurrentPoint();
	void DrawGlyphCurrentPoint(float vPreviewScale, ImVec2 vScreenPos, ImDrawList *vImDrawList);

public: // singleton
	static DebugPane *Instance()
	{
		static DebugPane *_instance = new DebugPane();
		return _instance;
	}

protected:
	DebugPane(); // Prevent construction
	DebugPane(const DebugPane&) {}; // Prevent construction by copying
	DebugPane& operator =(const DebugPane&) { return *this; }; // Prevent assignment
	~DebugPane(); // Prevent unwanted destruction};
};

#endif
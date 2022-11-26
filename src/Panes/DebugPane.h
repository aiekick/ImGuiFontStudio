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
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

private:
	void DrawDebugPane();
	void DrawDebugGlyphPane();

public:
	void SetGlyphToDebug(std::weak_ptr<GlyphInfos> vGlyphInfos);
	void Clear();

	ct::ivec2 GetGlyphCurrentPoint();
	void DrawGlyphCurrentPoint(float vPreviewScale, ImVec2 vScreenPos, ImDrawList *vImDrawList);

public: // singleton
	static std::shared_ptr<DebugPane> Instance()
	{
		static auto _instance = std::make_shared<DebugPane>();
		return _instance;
	}

public:
	DebugPane() = default; // Prevent construction
	DebugPane(const DebugPane&) = default; // Prevent construction by copying
	DebugPane& operator =(const DebugPane&) { return *this; }; // Prevent assignment
	~DebugPane() = default; // Prevent unwanted destruction};
};

#endif
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

#include <ctools/ConfigAbstract.h>
#include <Panes/Abstract/AbstractPane.h>

#include <imgui/imgui.h>
#include <Helper/SelectionHelper.h>

#include <functional>
#include <map>
#include <memory>
class ProjectFile;
class FontInfos;
class FontPreviewPane : public AbstractPane
{
private:
	std::function<int(ImGuiInputTextCallbackData*)> m_InputTextCallBack;
	ImVec4 m_GlyphButtonStateColor[3] = { ImVec4(), ImVec4(), ImVec4() };

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(int vWidgetId, std::string vUserDatas)  override;
	void DrawDialogsAndPopups(std::string vUserDatas) override;
	int DrawWidgets(int vWidgetId, std::string vUserDatas)  override;
	
private:
	void DrawFontPreviewPane();
	void DrawMixerWidget();
	void DrawMixedFontResult();

public: // singleton
	static FontPreviewPane *Instance()
	{
		static FontPreviewPane _instance;
		return &_instance;
	}

protected:
	FontPreviewPane(); // Prevent construction
	FontPreviewPane(const FontPreviewPane&) {}; // Prevent construction by copying
	FontPreviewPane& operator =(const FontPreviewPane&) { return *this; }; // Prevent assignment
	~FontPreviewPane(); // Prevent unwanted destruction};
};


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

#include <Panes/Abstract/AbstractPane.h>

#include <Helper/FontParser.h>

#include <string>

class ProjectFile;
class FontInfos;
class FontStructurePane : public AbstractPane
{
private:
	FontParser m_FontParser;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(int vWidgetId, std::string vUserDatas)  override;
	void DrawDialogsAndPopups(std::string vUserDatas) override;
	int DrawWidgets(int vWidgetId, std::string vUserDatas)  override;

private:
	void DrawFontStructurePane();
	void DisplayAnalyze();

public: // singleton
	static FontStructurePane *Instance()
	{
		static FontStructurePane _instance;
		return &_instance;
	}

protected:
	FontStructurePane(); // Prevent construction
	FontStructurePane(const FontStructurePane&) = default; // Prevent construction by copying
	FontStructurePane& operator =(const FontStructurePane&) { return *this; }; // Prevent assignment
	~FontStructurePane(); // Prevent unwanted destruction
};

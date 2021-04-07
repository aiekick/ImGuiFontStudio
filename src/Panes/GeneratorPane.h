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

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <stdint.h>
#include <string>
#include <map>

enum GeneratorStatusFlags
{
	GENERATOR_STATUS_NONE = 0,
	GENERATOR_STATUS_FONT_BATCH_ALLOWED = (1 << 1),
	GENERATOR_STATUS_FONT_MERGE_ALLOWED = (1 << 2),
	GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED = (1 << 3),
	GENERATOR_STATUS_DEFAULT =
		GENERATOR_STATUS_FONT_BATCH_ALLOWED |
		GENERATOR_STATUS_FONT_MERGE_ALLOWED |
		GENERATOR_STATUS_FONT_HEADER_GENERATION_ALLOWED
};

class ProjectFile;

class GeneratorPane : public AbstractPane
{
private: // STATUS FLAGS
	GeneratorStatusFlags m_GeneratorStatusFlags = GENERATOR_STATUS_DEFAULT;

public:
	void Init() override;
	void Unit() override;
	int DrawPanes(ProjectFile* vProjectFile, int vWidgetId) override;
	void DrawDialogsAndPopups(ProjectFile* vProjectFile) override;
	int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas) override;

	// STATUS FLAGS
	void AllowStatus(GeneratorStatusFlags vGeneratorStatusFlags);
	void ProhibitStatus(GeneratorStatusFlags vGeneratorStatusFlags);

private:
	void DrawGeneratorPane(ProjectFile *vProjectFile);
	void DrawFontsGenerator(ProjectFile *vProjectFile);
	void GeneratorFileDialogPane(const char *vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue);

	bool CheckAndDisplayGenerationConditions(ProjectFile *vProjectFile);
	void Show_BatchMode_PerFontSettings(ProjectFile* vProjectFile);
	void ModifyConfigurationAccordingToSelectedFeaturesAndErrors(ProjectFile* vProjectFile);

public: // singleton
	static GeneratorPane *Instance()
	{
		static GeneratorPane *_instance = new GeneratorPane();
		return _instance;
	}

protected:
	GeneratorPane(); // Prevent construction
	GeneratorPane(const GeneratorPane&) {}; // Prevent construction by copying
	GeneratorPane& operator =(const GeneratorPane&) { return *this; }; // Prevent assignment
	~GeneratorPane(); // Prevent unwanted destruction};
};


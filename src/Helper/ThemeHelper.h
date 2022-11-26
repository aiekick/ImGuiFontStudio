/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ctools/ConfigAbstract.h>
#include <imgui/imgui.h>
#include <string>
#include <map>

class ThemeHelper : public conf::ConfigAbstract
{
public:
#ifdef USE_SHADOW
	static float puShadowStrength; // low value is darker than higt (0.0f - 2.0f)
	static bool puUseShadow;
	static bool puUseTextureForShadow;
#endif
	bool puShowImGuiStyleEdtor = false;
	bool puShowTextEditorStyleEditor = false;

private:
	std::map<std::string, IGFD::FileStyle> prFileTypeInfos;
	ImGuiStyle prImGuiStyle;

public:
	void Draw();
	void DrawMenu();
	void ShowCustomImGuiStyleEditor(bool* vOpen, ImGuiStyle* ref = nullptr);
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

	ImGuiStyle GetImGuiStyle() { return prImGuiStyle; }

	void ApplyStyle();

private:
	void ApplyStyleColorsDefault();
	void ApplyStyleColorsOrangeBlue();
	void ApplyStyleColorsGreenBlue();
	void ApplyStyleColorsClassic();
	void ApplyStyleColorsDark();
	void ApplyStyleColorsLight();
	void ApplyStyleColorsDarcula();
	void ApplyStyleColorsRedDark();

	void ApplyFileTypeColors();

	std::string GetStyleColorName(ImGuiCol idx);
	int GetImGuiColFromName(const std::string& vName);

public: // singleton
	static ThemeHelper* Instance()
	{
		static ThemeHelper _instance;
		return &_instance;
	}

protected:
	ThemeHelper(); // Prevent construction
	ThemeHelper(const ThemeHelper&) = default; // Prevent construction by copying
	ThemeHelper& operator =(const ThemeHelper&) { return *this; }; // Prevent assignment
	~ThemeHelper(); // Prevent unwanted destruction
};
